#include <CowPi.h>
#include "interrupt_support.h"

#ifdef __AVR__

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

static void do_nothing(void) {}

void register_pin_ISR(uint32_t interrupt_mask, void (*isr)(void)) {
    cowpi_register_pin_ISR(interrupt_mask, isr);
}

//#define NUMBER_OF_PRESCALERS (7)
static unsigned int constexpr NUMBER_OF_PRESCALERS = 7;

struct timer_data {
    int prescalers[NUMBER_OF_PRESCALERS];
    uint32_t number_of_counter_values;
    uint8_t normal_mode_bits[2];
    uint8_t ctc_mode_bits[2];
    uint8_t clock_select_bits[2][NUMBER_OF_PRESCALERS];
    uint8_t number_of_isr_slots;
    void (*interrupt_service_routines[3])(void);
};

static struct timer_data timers[] = {
        {
                .prescalers = {1, 8, 64, 256, 1024, 0, 0},
                .number_of_counter_values = 1L << 8,
                .normal_mode_bits = {0, 0},
                .ctc_mode_bits = {1 << 1, 0},
                .clock_select_bits = {{0},
                                      {1, 2, 3, 4, 5, 0, 0}},
                .number_of_isr_slots = 0,
                .interrupt_service_routines = {do_nothing, do_nothing, do_nothing},
        },
        {
                .prescalers = {1, 8, 64, 256, 1024, 0, 0},
                .number_of_counter_values = 1L << 16,
                .normal_mode_bits = {0, 0},
                .ctc_mode_bits = {0, 1 << 3},
                .clock_select_bits = {{0},
                                      {1, 2, 3, 4, 5, 0, 0}},
                .number_of_isr_slots = 0,
                .interrupt_service_routines = {do_nothing, do_nothing, do_nothing},
        },
        {
                .prescalers = {1, 8, 32, 64, 128, 256, 1024},
                .number_of_counter_values = 1L << 8,
                .normal_mode_bits = {0, 0},
                .ctc_mode_bits = {1 << 1, 0},
                .clock_select_bits = {{0},
                                      {1, 2, 3, 4, 5, 6, 7}},
                .number_of_isr_slots = 0,
                .interrupt_service_routines = {do_nothing, do_nothing, do_nothing},
        }
};

ISR(TIMER1_OVF_vect) {
    timers[1].interrupt_service_routines[0]();
}

ISR(TIMER1_COMPA_vect) {
    timers[1].interrupt_service_routines[1]();
}

ISR(TIMER1_COMPB_vect) {
    timers[1].interrupt_service_routines[2]();
}

ISR(TIMER2_OVF_vect) {
    timers[2].interrupt_service_routines[0]();
}

ISR(TIMER2_COMPA_vect) {
    timers[2].interrupt_service_routines[1]();
}

ISR(TIMER2_COMPB_vect) {
    timers[2].interrupt_service_routines[2]();
}

float configure_timer(unsigned int timer_number, float desired_period_us) {
    if (timer_number < 1 || timer_number > 2) {
        // for now, we'll prohibit TIMER0 and assume only TIMER1 & TIMER2 exist -- later we can do uc-specific values
        return INFINITY;
    }
    if (desired_period_us < 1) {
        return INFINITY;
    }
    struct timer_data *timer = timers + timer_number;
    timer->interrupt_service_routines[0] = do_nothing;
    timer->interrupt_service_routines[1] = do_nothing;
    timer->interrupt_service_routines[2] = do_nothing;
    float const system_clock = 16.0;    // cycles per microsecond
    float best_error = INFINITY;
    float best_period = INFINITY;
    uint32_t best_count;
    int best_index;
    // Determine the best prescaler and comparison value
    for (unsigned i = 0; i < NUMBER_OF_PRESCALERS; i++) {
        int prescaler = timer->prescalers[i];
        if (prescaler != 0) {
            float closest_count_above = ceil(desired_period_us * system_clock / prescaler);
            float closest_count_below = floor(desired_period_us * system_clock / prescaler);
            float actual_period_us = (float) closest_count_above * prescaler / system_clock;
            float error = (float) fabs(actual_period_us - desired_period_us);
            // printf("prescaler = %d\tabove = %lu\terror = %u\n", prescaler, (uint32_t) closest_count_above, (unsigned int) error);
            if ((closest_count_above <= timer->number_of_counter_values) && (error < best_error)) {
                // printf("\tIMPROVEMENT\n");
                best_error = error;
                best_period = actual_period_us;
                best_count = ((uint32_t) closest_count_above) - 1;
                best_index = i;
            }
            actual_period_us = closest_count_below * prescaler / system_clock;
            error = (float) fabs(actual_period_us - desired_period_us);
            // printf("prescaler = %d\tbelow = %lu\terror = %u\n", prescaler, (uint32_t) closest_count_below, (unsigned int) error);
            if ((closest_count_below <= timer->number_of_counter_values) && (error < best_error)) {
                // printf("\tIMPROVEMENT\n");
                best_error = error;
                best_period = actual_period_us;
                best_count = ((uint32_t) closest_count_below) - 1;
                best_index = i;
            }
        }
    }
    // Configure the timer
    uint8_t *mode_bits;
    uint32_t compare_A;
    if (timer->number_of_counter_values - best_count == 1) {    // the comparison value is the maximum possible comparison value
        mode_bits = timer->normal_mode_bits;
        compare_A = 2 * best_count / 3;
        timer->number_of_isr_slots = 3;
    } else {
        mode_bits = timer->ctc_mode_bits;
        compare_A = best_count;
        timer->number_of_isr_slots = 2;
    }
    uint32_t compare_B = compare_A / 2;
    switch(timer_number) {
        case 1:
            TCCR1A = mode_bits[0] | timer->clock_select_bits[0][best_index];
            TCCR1B = mode_bits[1] | timer->clock_select_bits[1][best_index];
            TCCR1C = 0;
            TCNT1 = 0;
            OCR1A = compare_A;
            OCR1B = compare_B;
            TIMSK1 = 0;
            break;
        case 2:
            TCCR2A = mode_bits[0] | timer->clock_select_bits[0][best_index];
            TCCR2B = mode_bits[1] | timer->clock_select_bits[1][best_index];
            TCNT2 = 0;
            OCR2A = compare_A;
            OCR2B = compare_B;
            TIMSK2 = 0;
            break;
        default:
            // unreachable
            return INFINITY;
    }
    // printf("TCCR2 = %#04x,%02x\n", TCCR2B, TCCR2A);
    // printf("OCR2A = %#04x\n", OCR2A);
    return best_period;
}

bool register_timer_ISR(unsigned int timer_number, unsigned int isr_slot, void (*isr)(void)) {
    struct timer_data *timer = timers + timer_number;
    uint8_t number_of_isr_slots = timer->number_of_isr_slots;
    if (timer_number < 1 || timer_number > 2) {
        // for now, we'll prohibit TIMER0 and assume only TIMER1 & TIMER2 exist -- later we can do uc-specific values
        return false;
    }
    if (isr_slot >= number_of_isr_slots) {
        return false;
    }
    if (number_of_isr_slots == 2) {
        isr_slot++;
    }
    timer->interrupt_service_routines[isr_slot] = isr;
    // printf("updated timers[%u].interrupt_service_routines[%u]\n", timer_number, isr_slot);
    switch(timer_number) {
        case 1:
            TIMSK1 |= 1 << isr_slot;
            break;
        case 2:
            TIMSK2 |= 1 << isr_slot;
            break;
        default:
            // unreachable
            return false;
    }
    return true;
}

void reset_timer(unsigned int timer_number) {
    // for now, we'll prohibit TIMER0 and assume only TIMER1 & TIMER2 exist -- later we can do uc-specific values
    switch(timer_number) {
        case 1:
            TCNT1 = 0;
            break;
        case 2:
            TCNT2 = 0;
            break;
        default:
            // unreachable
            return;
    }
}

#ifdef __cplusplus
}
// extern "C"
#endif

#endif //__AVR__

#ifdef __MBED__
#include <InterruptIn.h>
#include <Ticker.h>

#ifdef __cplusplus
extern "C" {
#endif

static mbed::InterruptIn *inputs[32] = {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

void register_pin_ISR(uint32_t interrupt_mask, void (*isr)(void)) {
    int8_t i = 0;
    do {
        if (interrupt_mask & (1L << i)) {
            if (inputs[i] == nullptr) {
                inputs[i] = new mbed::InterruptIn((PinName)i, PullUp);
            }
            inputs[i]->disable_irq();   // disable interrupts while we're making changes
            inputs[i]->rise(isr);
            inputs[i]->fall(isr);
            inputs[i]->enable_irq();   // re-enable interrupts
        }
    } while (++i < 32);
}

//static mbed::Ticker *tickers[MAXIMUM_NUMBER_OF_TICKERS] = {
//        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
//};
//
//bool register_timer_ISR(uint32_t period_us, void (*isr)(void)) {
//    static int8_t number_of_tickers = 0;
//    if (number_of_tickers >= MAXIMUM_NUMBER_OF_TICKERS) {
//        return false;
//    }
//    tickers[number_of_tickers] = new mbed::Ticker();
//    tickers[number_of_tickers]->attach(isr, std::chrono::microseconds(period_us));
//    number_of_tickers++;
//    return true;
//}

struct timer_data {
    mbed::Ticker *ticker;
    std::chrono::microseconds period;
    void (*interrupt_service_routine)(void);
};

static std::chrono::microseconds constexpr no_time = std::chrono::microseconds(0);

static struct timer_data timers[MAXIMUM_NUMBER_OF_TIMERS] = {
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,},
        {.ticker = nullptr, .period = no_time, .interrupt_service_routine = nullptr,}
};

bool register_periodic_timer_ISR(unsigned int timer_number, uint32_t period_us, void (*isr)(void)) {
    if (timer_number >= MAXIMUM_NUMBER_OF_TIMERS) {
        return false;
    }
    if (timers[timer_number].ticker == nullptr) {
        timers[timer_number].ticker = new mbed::Ticker();
    }
    timers[timer_number].period = std::chrono::microseconds(period_us);
    timers[timer_number].interrupt_service_routine = isr;
    timers[timer_number].ticker->attach(timers[timer_number].interrupt_service_routine, timers[timer_number].period);
    return true;
}

void reset_periodic_timer(unsigned int timer_number) {
    if (timer_number >= MAXIMUM_NUMBER_OF_TIMERS) {
        return;
    }
    if (timers[timer_number].ticker == nullptr) {
        return;
    }
    timers[timer_number].ticker->detach();
    timers[timer_number].ticker->attach(timers[timer_number].interrupt_service_routine, timers[timer_number].period);
}

#ifdef __cplusplus
}
// extern "C"
#endif

#endif //__MBED__
