#include "mini_psu_demo.h"

#include <limits.h>
#include <string.h>

static uint16_t clampDuty(int32_t value, uint16_t minimum, uint16_t maximum)
{
    if (value < (int32_t)minimum)
    {
        return minimum;
    }
    if (value > (int32_t)maximum)
    {
        return maximum;
    }
    return (uint16_t)value;
}

static void resetTrace(MiniPsuDemoState *state)
{
    state->trace_length = 0U;
    (void)memset(state->trace, 0, sizeof(state->trace));
}

static void recordEvent(MiniPsuDemoState *state, MiniPsuDemoEvent event)
{
    if (state->trace_length < MINI_PSU_TRACE_CAPACITY)
    {
        state->trace[state->trace_length] = event;
        state->trace_length++;
    }
}

bool MiniPsuDemo_init(MiniPsuDemoState *state, const MiniPsuDemoConfig *config)
{
    if ((state == NULL) || (config == NULL))
    {
        return false;
    }
    if (config->minimum_duty_q15 > config->maximum_duty_q15)
    {
        return false;
    }
    if (config->maximum_duty_q15 > MINI_PSU_Q15_ONE)
    {
        return false;
    }
    if (config->nominal_duty_q15 < config->minimum_duty_q15 ||
        config->nominal_duty_q15 > config->maximum_duty_q15)
    {
        return false;
    }
    if (config->overcurrent_code <= config->adc_reference_code)
    {
        return false;
    }

    (void)memset(state, 0, sizeof(*state));
    state->config = *config;
    state->duty_q15 = config->nominal_duty_q15;
    return true;
}

uint16_t MiniPsuDemo_runControlCycle(
    MiniPsuDemoState *state,
    uint16_t adc_code,
    bool external_fault_active)
{
    int32_t correction;
    int32_t requested_duty;

    if (state == NULL)
    {
        return 0U;
    }

    resetTrace(state);
    recordEvent(state, MINI_PSU_EVENT_PWM_PERIOD);
    recordEvent(state, MINI_PSU_EVENT_ADC_SAMPLE);

    state->cycle_count++;
    state->last_adc_code = adc_code;
    state->last_error_code =
        (int32_t)state->config.adc_reference_code - (int32_t)adc_code;

    recordEvent(state, MINI_PSU_EVENT_ISR_ENTER);

    if (external_fault_active ||
        (adc_code >= state->config.overcurrent_code) ||
        state->trip_latched)
    {
        state->trip_latched = true;
        state->duty_q15 = 0U;
        recordEvent(state, MINI_PSU_EVENT_TRIP);
        return state->duty_q15;
    }

    correction =
        (state->last_error_code * (int32_t)state->config.proportional_gain_q15) >> 15;
    requested_duty = (int32_t)state->config.nominal_duty_q15 + correction;
    state->duty_q15 = clampDuty(
        requested_duty,
        state->config.minimum_duty_q15,
        state->config.maximum_duty_q15);

    recordEvent(state, MINI_PSU_EVENT_DUTY_UPDATE);
    return state->duty_q15;
}

bool MiniPsuDemo_clearTrip(
    MiniPsuDemoState *state,
    bool operator_authorized,
    bool hardware_fault_inactive)
{
    if (state == NULL)
    {
        return false;
    }
    if (!state->trip_latched)
    {
        return true;
    }
    if (!operator_authorized || !hardware_fault_inactive)
    {
        return false;
    }
    if (state->last_adc_code >= state->config.overcurrent_code)
    {
        return false;
    }

    state->trip_latched = false;
    state->duty_q15 = state->config.minimum_duty_q15;
    resetTrace(state);
    return true;
}
