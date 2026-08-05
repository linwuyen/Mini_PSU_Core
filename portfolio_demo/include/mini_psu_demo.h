#ifndef MINI_PSU_DEMO_H
#define MINI_PSU_DEMO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINI_PSU_Q15_ONE (32767U)
#define MINI_PSU_TRACE_CAPACITY (5U)

typedef enum
{
    MINI_PSU_EVENT_PWM_PERIOD = 1,
    MINI_PSU_EVENT_ADC_SAMPLE = 2,
    MINI_PSU_EVENT_ISR_ENTER = 3,
    MINI_PSU_EVENT_DUTY_UPDATE = 4,
    MINI_PSU_EVENT_TRIP = 5
} MiniPsuDemoEvent;

typedef struct
{
    uint16_t adc_reference_code;
    uint16_t overcurrent_code;
    uint16_t nominal_duty_q15;
    uint16_t minimum_duty_q15;
    uint16_t maximum_duty_q15;
    int16_t proportional_gain_q15;
} MiniPsuDemoConfig;

typedef struct
{
    MiniPsuDemoConfig config;
    uint16_t duty_q15;
    uint16_t last_adc_code;
    int32_t last_error_code;
    uint32_t cycle_count;
    bool trip_latched;
    MiniPsuDemoEvent trace[MINI_PSU_TRACE_CAPACITY];
    size_t trace_length;
} MiniPsuDemoState;

bool MiniPsuDemo_init(MiniPsuDemoState *state, const MiniPsuDemoConfig *config);

uint16_t MiniPsuDemo_runControlCycle(
    MiniPsuDemoState *state,
    uint16_t adc_code,
    bool external_fault_active);

bool MiniPsuDemo_clearTrip(
    MiniPsuDemoState *state,
    bool operator_authorized,
    bool hardware_fault_inactive);

#ifdef __cplusplus
}
#endif

#endif
