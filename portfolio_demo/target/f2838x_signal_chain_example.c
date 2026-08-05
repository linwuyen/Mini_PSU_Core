/*
 * Public-safe F2838x integration skeleton.
 *
 * This file is intentionally excluded from the portable host build. It shows
 * where the verified logic attaches to a SysConfig/DriverLib project; board,
 * pin, polarity, ADC channel, scaling, CMPSS source, dead-time, and trip paths
 * must be reviewed for the actual hardware before enabling PWM.
 */

#if defined(MINI_PSU_C2000_TARGET_EXAMPLE)

#include "driverlib.h"
#include "device.h"
#include "mini_psu_demo.h"

#define MINI_PSU_PWM_BASE EPWM1_BASE
#define MINI_PSU_ADC_RESULT_BASE ADCARESULT_BASE
#define MINI_PSU_ADC_SOC ADC_SOC_NUMBER0
#define MINI_PSU_ISR_MARKER_GPIO 31U

static MiniPsuDemoState g_demoState;

void MiniPsuTarget_initializeReference(void)
{
    const MiniPsuDemoConfig config = {
        .adc_reference_code = 2000U,
        .overcurrent_code = 3500U,
        .nominal_duty_q15 = 16384U,
        .minimum_duty_q15 = 1000U,
        .maximum_duty_q15 = 30000U,
        .proportional_gain_q15 = 8192
    };

    /*
     * Expected SysConfig ownership:
     *   ePWM1 period and complementary output polarity/dead-time
     *   ePWM1 SOCA trigger point
     *   ADCA SOC0 channel/acquisition window
     *   ADCA1 interrupt routing
     *   CMPSS/XBAR/Trip-Zone hardware shutdown path
     *   ISR timing-marker GPIO
     */
    (void)MiniPsuDemo_init(&g_demoState, &config);
}

__interrupt void MiniPsuTarget_adca1Isr(void)
{
    uint16_t adcCode;
    uint16_t dutyQ15;
    uint16_t compareValue;
    bool hardwareFaultActive;

    GPIO_writePin(MINI_PSU_ISR_MARKER_GPIO, 1U);

    adcCode = ADC_readResult(MINI_PSU_ADC_RESULT_BASE, MINI_PSU_ADC_SOC);

    /* Replace with the reviewed native CMPSS/XBAR status for the target. */
    hardwareFaultActive = false;

    dutyQ15 = MiniPsuDemo_runControlCycle(
        &g_demoState,
        adcCode,
        hardwareFaultActive);

    if (g_demoState.trip_latched)
    {
        EPWM_forceTripZoneEvent(MINI_PSU_PWM_BASE, EPWM_TZ_FORCE_EVENT_OST);
    }
    else
    {
        const uint16_t period = EPWM_getTimeBasePeriod(MINI_PSU_PWM_BASE);
        compareValue = (uint16_t)(((uint32_t)period * dutyQ15) / MINI_PSU_Q15_ONE);
        EPWM_setCounterCompareValue(
            MINI_PSU_PWM_BASE,
            EPWM_COUNTER_COMPARE_A,
            compareValue);
    }

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    GPIO_writePin(MINI_PSU_ISR_MARKER_GPIO, 0U);
}

#endif
