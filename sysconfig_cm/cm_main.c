
//
// Included Files
//
#include "driverlib_cm.h"
#include "cm.h"

uint16_t CMcnt;

//
// Main
//
void main(void)
{
    //
    // Initialize device clock and peripherals for CM core
    //
    CM_init();

    //
    // Loop Forever
    //
    for(;;)
    {
        //
        // Add CM application code here
        //
        CMcnt++;
    }
}

//
// End of File
//
