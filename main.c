#include "msp.h"
#include "driverlib/MSP432P4xx/driverlib.h"
#include "_libraries/fsm.h"
#include "_libraries/interruptHandlers.h"

#define NUM_STATES 4;

/*
 * Maglie Francesco
 * Recruiting project for E-Agle
 */

State_t current_state = INIT;

Machine_t fsm[4] = {
                 {INIT, sfm_init},
                 {RUNNING, sfm_running},
                 {DANGER, sfm_danger},
                 {EMERGENCY, sfm_emergency}
};

//(*(fsm[0].state_function))();

void main(void)
{
    WDT_A_holdTimer();

    sfm_init();

    __sleep();

}

