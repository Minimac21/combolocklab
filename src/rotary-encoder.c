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
#include "interrupt_support.h"
#include "rotary-encoder.h"

#define A_WIPER_PIN         (16)
#define B_WIPER_PIN         (A_WIPER_PIN + 1)

typedef enum {
    HIGH_HIGH, HIGH_LOW, LOW_LOW, LOW_HIGH, UNKNOWN
} rotation_state_t;

static rotation_state_t volatile state;
static direction_t volatile direction = STATIONARY;
static int volatile clockwise_count = 0;
static int volatile counterclockwise_count = 0;

static void handle_quadrature_interrupt();

void initialize_rotary_encoder() {
    cowpi_set_pullup_input_pins((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN));
    ;
//    register_pin_ISR((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN), handle_quadrature_interrupt);
}

uint8_t get_quadrature() {
    ;
    return 0;
}

char *count_rotations(char *buffer) {
    ;
    return buffer;
}

direction_t get_direction() {
    ;
    return STATIONARY;
}

static void handle_quadrature_interrupt() {
    static rotation_state_t last_state = UNKNOWN;
    uint8_t quadrature = get_quadrature();
    ;
}