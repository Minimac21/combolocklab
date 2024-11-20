#ifndef INTERRUPT_SUPPORT_H
#define INTERRUPT_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
* @brief Registers a function to service pin-based interrupts triggered by
* logic-level changes on one or more pins.
*
* The registered function will be invoked whenever there is a low-to-high or
* a high-to-low change. If behavior is only required for a rising edge or for
* a falling edge, or if the behavior for rising and falling edges must
* differ, then the function should have a conditional to determine the
* direction of the change.
*
* If the change is generated by a mechanical device, then the function is
* responsible for debouncing if there is not a hardware debouncing circuit.
*
* The <code>interrupt_mask</code> argument is used to specify which pins will
* be serviced by the registered function. Bit0 corresponds to Pin 0, Bit1
* corresponds to Pin 1, and so on. A 1 in a particular bit indicates that the
* function is to be registered for changes on the corresponding pin. If more
* than one bit has a 1, then the function will be registered for each of the
* corresponding pins. If there previously was a function registered to handle
* changes on a specified pin, then the new function will replace the old
* function. A bit with a 0 signifies nothing more than that the function is
* not being registered to service changes on that pin at this time.
*
* @param interrupt_mask A bit vector specifying which pins will be serviced by
*      the registered ISR
* @param isr The function that will service interrupts triggered by changes on
*      the specified pins
*/
void register_pin_ISR(uint32_t interrupt_mask, void (*isr)(void));

/**
 * @brief Sets a timer to the beginning of its interrupt period.
 *
 * @note On AVR architectures, this affects all ISR slots for the specified
 *      timer.
 *
 * If the interrupt period is *T* microseconds, then:
 * <ul>
 * <li> On AVR architectures, the ISR in slot 0 will execute *T* microseconds
 *      after `reset_timer()` returns. ISRs in other slots will fire sooner,
 *      according to their place in the timer period's partitioning.
 * <li> On MBED systems, the ISR will execute *T* microseconds after
 *      `reset_timer()` returns.
 * </ul>
 *
 * @param timer_number The timer whose counter is to be reset
 */
void reset_periodic_timer(unsigned int timer_number);

#ifdef __AVR__

/**
 * @brief Configures an AVR timer.
 *
 * The timer will be configured to reset its counter at or near the specified
 * interval. The function will select the configuration that has an actual
 * period as close as possible to the specified period.
 *
 * The timer might be configured for Normal mode or for CTC mode;
 * if a Normal mode configuration and a CTC mode configuration are
 * equally-accurate then the timer will be configured in Normal mode.
 * WGM mode is not supported.
 *
 * Any ISRs that had previously been registered for the timer will be
 * deregistered.
 *
 * This function may not be used to configure TIMER0.
 *
 * @param timer_number The timer to be configured
 * @param desired_period_us The preferred interrupt period
 * @return The actual interrupt period
 */
float configure_timer(unsigned int timer_number, float desired_period_us);

/**
 * @brief Registers a function to service timer interrupts.
 *
 * Each timer has ISR slots for two or three interrupt service routines:
 * two if the timer is in CTC mode, three if the timer is in Normal mode.
 * Regardless of the timer mode, the ISR registered for slot 0 will execute
 * when the counter resets. If the timer is in CTC mode, then the ISR registered
 * for slot 1 will execute halfway between counter resets. If the timer is in
 * Normal mode, then the ISRs registered for slots 1 and 2 will partition the
 * timer period into thirds.
 *
 * The timer must have previously been registered using
 * <code>configure_timer()</code>.
 *
 * This function may not be used to register ISRs for TIMER0.
 *
 * @param timer_number The timer whose interrupt will invoke the ISR
 * @param isr_slot 0 if the ISR should be invoked when the counter resets,
 *      1 or 2 otherwise
 * @param isr The function that will service the timer's interrupts
 * @return <code>true</code> if the ISR was successfully registered;
 *      <code>false</code> otherwise
 */
bool register_timer_ISR(unsigned int timer_number, unsigned int isr_slot, void (*isr)(void));

#endif //__AVR__

#ifdef __MBED__

//static unsigned int constexpr MAXIMUM_NUMBER_OF_TICKERS = 8;
#define MAXIMUM_NUMBER_OF_TIMERS (8)     // gotta maintain portability with pre-C23 for now

/**
 * @brief Configures a timer interrupt to fire, and assigns a function to
 * service that interrupt.
 *
 * This function supports up to `MAXIMUM_NUMBER_OF_TIMERS` timers.
 *
 * Any ISR that had previously been registered for the timer will be
 * deregistered.
 *
 * @param timer_number A unique handle for the virtual timer being configured
 * @param period_us The specified interrupt period
 * @param isr The function that will service the timer's interrupts
 * @return <code>true</code> if the periodic interrupt was successfully
 *      configured and the ISR was successfully registered; <code>false</code>
 *      otherwise
 */
bool register_periodic_timer_ISR(unsigned int timer_number, uint32_t period_us, void (*isr)(void));

#endif //__MBED__

#ifdef __cplusplus
} // extern "C"
#endif

#endif //INTERRUPT_SUPPORT_H
