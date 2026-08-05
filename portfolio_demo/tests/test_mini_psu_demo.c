#include "mini_psu_demo.h"

#include <assert.h>
#include <stdio.h>

static MiniPsuDemoConfig defaultConfig(void)
{
    MiniPsuDemoConfig config = {
        .adc_reference_code = 2000U,
        .overcurrent_code = 3500U,
        .nominal_duty_q15 = 16384U,
        .minimum_duty_q15 = 1000U,
        .maximum_duty_q15 = 30000U,
        .proportional_gain_q15 = 8192
    };
    return config;
}

static void assertNormalTrace(const MiniPsuDemoState *state)
{
    assert(state->trace_length == 4U);
    assert(state->trace[0] == MINI_PSU_EVENT_PWM_PERIOD);
    assert(state->trace[1] == MINI_PSU_EVENT_ADC_SAMPLE);
    assert(state->trace[2] == MINI_PSU_EVENT_ISR_ENTER);
    assert(state->trace[3] == MINI_PSU_EVENT_DUTY_UPDATE);
}

static void testNominalControl(void)
{
    MiniPsuDemoState state;
    MiniPsuDemoConfig config = defaultConfig();

    assert(MiniPsuDemo_init(&state, &config));
    assert(MiniPsuDemo_runControlCycle(&state, 1800U, false) > config.nominal_duty_q15);
    assert(state.last_error_code == 200);
    assert(state.cycle_count == 1U);
    assert(!state.trip_latched);
    assertNormalTrace(&state);

    assert(MiniPsuDemo_runControlCycle(&state, 2200U, false) < config.nominal_duty_q15);
    assert(state.last_error_code == -200);
    assertNormalTrace(&state);
}

static void testDutyClamp(void)
{
    MiniPsuDemoState state;
    MiniPsuDemoConfig config = defaultConfig();
    config.proportional_gain_q15 = 32767;

    assert(MiniPsuDemo_init(&state, &config));
    assert(MiniPsuDemo_runControlCycle(&state, 0U, false) == config.maximum_duty_q15);
    assert(MiniPsuDemo_runControlCycle(&state, 3400U, false) == config.minimum_duty_q15);
}

static void testTripLatchAndAuthorizedClear(void)
{
    MiniPsuDemoState state;
    MiniPsuDemoConfig config = defaultConfig();

    assert(MiniPsuDemo_init(&state, &config));
    assert(MiniPsuDemo_runControlCycle(&state, config.overcurrent_code, false) == 0U);
    assert(state.trip_latched);
    assert(state.trace_length == 4U);
    assert(state.trace[3] == MINI_PSU_EVENT_TRIP);

    assert(MiniPsuDemo_runControlCycle(&state, 1000U, false) == 0U);
    assert(state.trip_latched);
    assert(!MiniPsuDemo_clearTrip(&state, false, true));
    assert(!MiniPsuDemo_clearTrip(&state, true, false));
    assert(MiniPsuDemo_clearTrip(&state, true, true));
    assert(!state.trip_latched);
    assert(state.duty_q15 == config.minimum_duty_q15);

    assert(MiniPsuDemo_runControlCycle(&state, 1900U, false) > 0U);
    assertNormalTrace(&state);
}

static void testExternalFault(void)
{
    MiniPsuDemoState state;
    MiniPsuDemoConfig config = defaultConfig();

    assert(MiniPsuDemo_init(&state, &config));
    assert(MiniPsuDemo_runControlCycle(&state, 1000U, true) == 0U);
    assert(state.trip_latched);
    assert(state.trace[3] == MINI_PSU_EVENT_TRIP);
}

static void testInvalidConfiguration(void)
{
    MiniPsuDemoState state;
    MiniPsuDemoConfig config = defaultConfig();

    config.minimum_duty_q15 = 30001U;
    assert(!MiniPsuDemo_init(&state, &config));

    config = defaultConfig();
    config.overcurrent_code = config.adc_reference_code;
    assert(!MiniPsuDemo_init(&state, &config));
}

int main(void)
{
    testNominalControl();
    testDutyClamp();
    testTripLatchAndAuthorizedClear();
    testExternalFault();
    testInvalidConfiguration();
    puts("PASS: Mini PSU vertical control reference");
    return 0;
}
