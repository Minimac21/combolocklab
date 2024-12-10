/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author (STUDENTS -- TYPE YOUR NAME HERE)
 * @author (STUDENTS -- TYPE YOUR NAME HERE)
 *
 * @brief Code to determine the direction that a rotary encoder is turning.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "display.h"
#include "interrupt_support.h"
#include "rotary-encoder.h"

#define A_WIPER_PIN         (16)
#define B_WIPER_PIN         (A_WIPER_PIN + 1)

typedef enum {
    HIGH_HIGH, HIGH_LOW, LOW_LOW, LOW_HIGH, UNKNOWN
} rotation_state_t;

static cowpi_ioport_t volatile *ioport = (cowpi_ioport_t*) (0xD0000000);
static rotation_state_t volatile state = HIGH_HIGH;
static direction_t volatile direction = STATIONARY;
static int volatile clockwise_count = 0;
static int volatile counterclockwise_count = 0;
char count_buffer[20];

static void handle_quadrature_interrupt();

void initialize_rotary_encoder() {
    ioport = (cowpi_ioport_t *) (0xD0000000);
    cowpi_set_pullup_input_pins((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN));
    ;
    register_pin_ISR((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN), handle_quadrature_interrupt);
    switch (get_quadrature()){
        case 0b00:
            state = LOW_LOW;
            break;
        case 0b01:
            state = LOW_HIGH;
            break;
        case 0b10:
            state = HIGH_LOW;
            break;
        case 0b11:
            state = HIGH_HIGH;
            break;
    }
}

uint8_t get_quadrature() {
    uint32_t input = ioport->input;
    input = (input & (1<<A_WIPER_PIN | 1<<B_WIPER_PIN));
    uint8_t result = (uint8_t) (input>>A_WIPER_PIN);
    return result;
}

int get_cw_count(){
    return clockwise_count;
}
int get_ccw_count(){
    return counterclockwise_count;
}

void set_cw_count(int x){
    clockwise_count = x;
}
void set_ccw_count(int x){
    counterclockwise_count = x;
}

char *count_rotations(char *buffer) {
    sprintf(buffer, "CW: %d CCW: %d", clockwise_count, counterclockwise_count);
    return buffer;
}

direction_t get_direction() {
    direction_t curr_direction = direction; 
    direction = STATIONARY;
    return curr_direction;
}

static void handle_quadrature_interrupt() {
    static rotation_state_t last_state = HIGH_HIGH;
    uint8_t quadrature = get_quadrature();
    switch(state){
        case HIGH_HIGH:
            switch(quadrature){
                case 0b10:
                    last_state = state;
                    state = HIGH_LOW;
                    break;
                case 0b01:
                    last_state = state;
                    state = LOW_HIGH;
                    break;
            }
            break;
        case HIGH_LOW:
            switch(quadrature){
                case 0b11:
                    last_state = state;
                    state = HIGH_HIGH;
                    break;
                case 0b00:
                    if(last_state==HIGH_HIGH){
                        last_state = state;
                        state = LOW_LOW;
                        direction = CLOCKWISE;
                        clockwise_count++;
                        // count_rotations(count_buffer);
                        // display_string(4, count_buffer);
                    }
                    break;
            }
            break;
        case LOW_HIGH:
            switch(quadrature){
                case 0b11:
                    last_state = state;
                    state = HIGH_HIGH;
                    break;
                case 0b00:
                    if(last_state==HIGH_HIGH){
                        last_state = state;
                        state = LOW_LOW;
                        direction = COUNTERCLOCKWISE;
                        counterclockwise_count++;
                        // count_rotations(count_buffer);
                        // display_string(4, count_buffer);
                    }
                    break;
            }
            break;
        case LOW_LOW:
            switch(quadrature){
                case 0b01:
                    if(last_state==HIGH_LOW){
                        last_state = state;
                        state = LOW_HIGH;
                    }
                    break;
                case 0b10:
                    if(last_state==LOW_HIGH){
                        last_state = state;
                        state = HIGH_LOW;
                    }
                    break;
            }
    }
}