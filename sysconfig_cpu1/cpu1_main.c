//
// Included Files
//
// Make sure to include "board.h" to use SysConfig
//
#include "driverlib.h"
#include "device.h"
#include "board.h"

uint16_t CPU1HB;
//
// Main
//
void main(void)
{
    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Boot CPU2 and CM cores (guarded for v0.1-A)
    //
#ifdef ENABLE_MULTICORE_BOOT
    //
    // Boot CPU2 core
    //
    Device_bootCPU2(BOOT_MODE_CPU2);

    //
    // Boot CM (Cortex-M4) core
    //
#ifdef _FLASH
    Device_bootCM(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    Device_bootCM(BOOTMODE_BOOT_TO_S0RAM);
#endif
#endif // ENABLE_MULTICORE_BOOT

    //
    // Initialize GPIO
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Initialize settings from SysConfig
    //
    Board_init();

    //
    // Enable Time-Base Clock (TBCLK) to start all configured ePWM counters
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    //
    // Loop Forever
    //
    for(;;)
    {
        //
        // Add CPU1 application code here
        //
        CPU1HB++;

    }
}

//
// End of File
//
