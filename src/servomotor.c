/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author Luke Doughty
 * @author Mac DePriest
 *
 * @brief Code to control a servomotor.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "servomotor.h"
#include "interrupt_support.h"

#define SERVO_PIN           (22)
#define PULSE_INCREMENT_uS  (500) // pulse varies from 500 (fully left) to 2500 (fully right). 1500 is center
#define SIGNAL_PERIOD_uS    (20000)
static int volatile pulse_width_us;
static cowpi_ioport_t volatile *ioport;
static void handle_timer_interrupt();

void initialize_servo() {
    ioport = (cowpi_ioport_t *) (0xD0000000);
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer) {
    if (cowpi_left_button_is_pressed()){
        strcpy(buffer, "Servo: center");
        center_servo();
    } else if (cowpi_left_switch_is_in_right_position()) {
        strcpy(buffer, "Servo: right");
        rotate_full_counterclockwise();
    } else { // left switch in left position
        strcpy(buffer, "Servo: left");
        rotate_full_clockwise();
    }
    return buffer;
}

void center_servo() {
    pulse_width_us = 1500;
}

void rotate_full_clockwise() {
    pulse_width_us = 500;
}

void rotate_full_counterclockwise() {
    pulse_width_us = 2500;
}

void writeToServo(int turn_on) {
    if (turn_on) {
        ioport->output |= (1 << SERVO_PIN);
    } else {
        ioport->output &= ~(1 << SERVO_PIN);
    }
}

static void handle_timer_interrupt() {
    static int time_until_next_rising_edge = PULSE_INCREMENT_uS;
    static int time_until_next_falling_edge = PULSE_INCREMENT_uS;
    time_until_next_rising_edge -= PULSE_INCREMENT_uS ;
    time_until_next_falling_edge -= PULSE_INCREMENT_uS;
    if (time_until_next_rising_edge == 0){
        writeToServo(1);
        time_until_next_rising_edge = SIGNAL_PERIOD_uS;
        time_until_next_falling_edge = pulse_width_us;
    } else if (time_until_next_falling_edge == 0){
        writeToServo(0);
    }
}
