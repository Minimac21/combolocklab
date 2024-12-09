/**************************************************************************//**
 *
 * @file lock-controller.c
 *
 * @author (STUDENTS -- TYPE YOUR NAME HERE)
 * @author (STUDENTS -- TYPE YOUR NAME HERE)
 *
 * @brief Code to implement the "combination lock" mode.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "display.h"
#include "lock-controller.h"
#include "rotary-encoder.h"
#include "servomotor.h"

#define LEFT_LED_PIN    (21)
#define RIGHT_LED_PIN   (20)

static uint8_t combination[3] __attribute__((section (".uninitialized_ram.")));

typedef enum {
    LOCKED, UNLOCKED, CHANGING, ALARMED
} state_t;
state_t state;
uint8_t working_index = 0;
uint8_t attempt_number = 0;
uint8_t entered_combo_array[3];
char count_buffer[20];
char combo_buffer[20];
static cowpi_timer_t volatile *timer;

uint8_t const *get_combination() {
    for (int i = 0; i < 3; i++){
        combination[i] %= 16;
    }
    return combination;
}

uint32_t get_microseconds() {
    return timer->raw_lower_word;
}

void sleep_quarter_second(){
    // TODO: Make this not freeze display.
    uint32_t wait_start_time = get_microseconds();
    while (get_microseconds() < 250000 + wait_start_time){;;}
}

void force_combination_reset() {
    combination[0] = 5;
    combination[1] = 10;
    combination[2] = 15;
}

void set_system_to_locked(uint8_t *combination){
    digitalWrite(LEFT_LED_PIN, 1);
    digitalWrite(RIGHT_LED_PIN, 0);
    rotate_full_clockwise();
    
    // SHOW THE COMBINATION (delete later?)
    static char combo_buffer[22] = {0};
    sprintf(combo_buffer, "Combo: %02d-%02d-%02d", combination[0], combination[1], combination[2]);
    display_string(3, combo_buffer);
}

void show_system_as_unlocked(){
    digitalWrite(LEFT_LED_PIN, 0);
    digitalWrite(RIGHT_LED_PIN, 1);
    rotate_full_counterclockwise();
    display_string(1, "OPEN");
}

void show_bad_try(int attempt_num){
    char buffer[11];
    sprintf(buffer, "bad try %d", attempt_num);
    display_string(2, buffer);

    // LEDs blink twice
    digitalWrite(LEFT_LED_PIN, 1);
    digitalWrite(RIGHT_LED_PIN, 1);
    sleep_quarter_second();
    digitalWrite(LEFT_LED_PIN, 0);
    digitalWrite(RIGHT_LED_PIN, 0);
    sleep_quarter_second();
    digitalWrite(LEFT_LED_PIN, 1);
    digitalWrite(RIGHT_LED_PIN, 1);
    sleep_quarter_second();
    digitalWrite(LEFT_LED_PIN, 0);
    digitalWrite(RIGHT_LED_PIN, 0);
}

void show_system_as_alarmed(){
    display_string(1, "alert!");
    digitalWrite(LEFT_LED_PIN, 1);
    digitalWrite(RIGHT_LED_PIN, 1);
    sleep_quarter_second();
    digitalWrite(LEFT_LED_PIN, 0);
    digitalWrite(RIGHT_LED_PIN, 0);
    sleep_quarter_second();
}

void show_system_as_changing(){
    display_string(1, "enter!");
}

void initialize_lock_controller() {
    timer = (cowpi_timer_t *) (0x40054000);
    set_system_to_locked(get_combination());
    state = LOCKED;
}

bool arraysAreEqual(uint8_t size_of_array, uint8_t* array1, uint8_t* array2){
    // WARNING does not check for out of bounds.
    bool equal_so_far = true;
    for (int i = 0; i < size_of_array; i++){
        if (equal_so_far && (array1[i] != array2[i])){
            equal_so_far = false;
        }
    }
    return equal_so_far;
}

void reset_combo_entry(){
    int COMBO_LENGTH = 3;
    for (int i = 0; i < COMBO_LENGTH; i++){
        entered_combo_array[i] = 0;
    }
    set_cw_count(0);
    set_ccw_count(0);
    working_index = 0;
}

void get_combo_input(uint8_t entered_combo_array[]){
    int COMBO_LENGTH = 3; // needs to be equal to length of entered_combo_array[].
    // first and third numbers are clockwise, second number is counterclockwise.
    direction_t expected_direction = (working_index % 2 == 0) ? CLOCKWISE : COUNTERCLOCKWISE;
    direction_t actual_direction = get_direction();
    if ((actual_direction != STATIONARY) && (actual_direction != expected_direction)){ // when we switch directions.
        working_index++;
        set_cw_count(0);
        set_ccw_count(0);
    } else { // still spinning. looking for next number.
        bool valid = true;
        if (expected_direction == CLOCKWISE){
            int cw_count = get_cw_count();
            entered_combo_array[working_index] = cw_count;
            valid = cw_count <= ((COMBO_LENGTH - working_index) * 16); // check if you overspun and missed your number.
        } else if (expected_direction == COUNTERCLOCKWISE){ // this 'if' is redundent, but i kept it for clarity because "STATIONARY" exists.
            int ccw_count = get_ccw_count();
            entered_combo_array[working_index] = ccw_count;
            valid = ccw_count <= ((COMBO_LENGTH - working_index) * 16);
        }
        if (!valid){
            reset_combo_entry();
            attempt_number++;
            show_bad_try(attempt_number);
        }
    }

    count_rotations(count_buffer);
    display_string(4, count_buffer);
    char workingIndexString[5];
    sprintf(workingIndexString, "%d", working_index);
}

bool get_new_combo_from_keypad(){
    // STUB: return 1 if successfully changed password, else 0.
    return 0;
}

void format_combo_string(char* combo_buffer, uint8_t* combo_array){
    char formatted_combo[3][3];
    for (int i = 0; i < 3; i++) {
        if (combo_array[i] == 0) {
            strcpy(formatted_combo[i], "  ");
        } else {
            sprintf(formatted_combo[i], "%02d", combo_array[i]);
        }
    }
    sprintf(combo_buffer, "Combo: %s-%s-%s", formatted_combo[0], formatted_combo[1], formatted_combo[2]);
}


void control_lock() {
    // int clockwise_count = get_cw_count();
    // int counterclockwise_count = get_ccw_count();
    switch (state){
        case LOCKED:
            get_combo_input(entered_combo_array);
            if (working_index >= 3) { // filled in all three numbers.
                if (arraysAreEqual(3, entered_combo_array, get_combination())){ // the passcode is correct.
                    state = UNLOCKED;
                    clear_display();
                } else { // passcode was wrong.
                    reset_combo_entry();
                    attempt_number++;
                    show_bad_try(attempt_number);
                }
            }
            if (attempt_number > 3){
                state = ALARMED;
            }
            format_combo_string(combo_buffer, entered_combo_array);
            display_string(1, combo_buffer);
            break;
        case UNLOCKED:
            show_system_as_unlocked();
            if (cowpi_left_switch_is_in_right_position() && cowpi_right_button_is_pressed()){
                state = CHANGING;
                clear_display();
            }
            break;
        case CHANGING:
            show_system_as_changing();
            get_new_combo_from_keypad(); // stub. todo.
            break;
        case ALARMED:
            show_system_as_alarmed();
            break;
    }
    char state_buffer[5];
    sprintf(state_buffer, "State: %d", state);
    display_string(5, state_buffer);

}
