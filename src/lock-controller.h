/*                       *
 * DO NOT EDIT THIS FILE *
 *                       */

/**************************************************************************//**
 *
 * @file lock-controller.h
 *
 * @author Christopher A. Bohn
 *
 * @brief Function prototypes that the lock controller needs to make available
 *      to combolock.c
 *
 ******************************************************************************/

/*
 * ComboLock Group Project (c) 2022-24 Christopher A. Bohn
 *
 * Starter code licensed under the Apache License, Version 2.0
 * (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <stdint.h>

#ifndef COMBOLOCK_LOCK_CONTROLLER_H
#define COMBOLOCK_LOCK_CONTROLLER_H

uint8_t const *get_combination();
void force_combination_reset();
void initialize_lock_controller();
void control_lock();

#endif //COMBOLOCK_LOCK_CONTROLLER_H
