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
    LOCKED, UNLOCKED, CHANGING
} state_t;
typedef enum {
    FIRST, SECOND, THIRD
} num_t;
state_t state;
num_t working_index;
int working_number;
bool valid = true;
static int rotations_count = 0;
int first_num=0;
int second_num=0;
int third_num=0;
char count_buffer[20];
char combo_buffer[20];


uint8_t const *get_combination() {
    return combination;
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
    display_string(1, "LOCKED");
    state = LOCKED;
    working_index = FIRST;
    
    // SHOW THE COMBINATION (delete later?)
    static char combo_buffer[22] = {0};
    sprintf(combo_buffer, "Combo: %02d-%02d-%02d", combination[0], combination[1], combination[2]);
    display_string(3, combo_buffer);
}

void initialize_lock_controller() {
    set_system_to_locked(combination);
}


void control_lock() {
    int clockwise_count = get_cw_count();
    int counterclockwise_count = get_ccw_count();
    switch (state){
        case LOCKED:
            switch(working_index){
                case FIRST:
                    first_num = clockwise_count % 16;
                    // on to second num
                    if(counterclockwise_count){
                        rotations_count = clockwise_count / 16;  
                        if(rotations_count < 3){
                            valid = false;
                        }
                        working_index = SECOND;
                        set_cw_count(0);
                        count_rotations(count_buffer);
                        display_string(4, count_buffer);
                    }
                    break;
                case SECOND:
                    second_num = counterclockwise_count % 16;
                    if(clockwise_count){
                        // on to third num
                        rotations_count = counterclockwise_count / 16;  
                        if(rotations_count < 2){
                            valid = false;
                        }
                        working_index = THIRD;
                        set_ccw_count(0);
                        count_rotations(count_buffer);
                        display_string(4, count_buffer);
                    } else if(counterclockwise_count >= 16 * 3){
                        first_num = 0;
                        second_num = 0;
                        working_index = FIRST;
                    }
                    break;
                case THIRD:
                    third_num = clockwise_count % 16;
                    if(counterclockwise_count){
                        // req 12f, clear combo
                        first_num = 0;
                        second_num = 0;
                        third_num = 0;
                        working_index = FIRST;
                        set_ccw_count(0);
                        set_cw_count(0);
                        count_rotations(count_buffer);
                        display_string(4, count_buffer);
                    } else if(counterclockwise_count >= 16 * 2){
                        first_num = 0;
                        second_num = 0;
                        third_num = 0;
                        set_ccw_count(0);
                        set_cw_count(0);
                        working_index = FIRST;
                    }
            }
            break;
        case UNLOCKED:
            break;
        case CHANGING:
            break;
    }
    sprintf(combo_buffer, "Combo: %02d-%02d-%02d", first_num, second_num, third_num);
    display_string(1, combo_buffer);
    ;
}
