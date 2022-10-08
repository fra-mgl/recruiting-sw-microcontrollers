/*
 * fsm.c
 */

#include "_libraries/fsm.h"
#include "driverlib/MSP432P4xx/driverlib.h"



const Timer_A_UpModeConfig upModeConf200 = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_10,
    60000, // 3*10^6 * 200 *10^-3 / 10
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

const Timer_A_UpModeConfig upModeConf350 = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_5,
    21000, //3*10^6 * 350 *10^-3 / 5
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};


void sfm_init(){
    /*
     * configure LEDs
     * configure sensors
     * configure both timers and start them
     */

    /*
     * LEDs
     * red led is set to indicate OVERVOLTAGE
     * blue led is set to indicate UNDERVOLTAGE
     */

    GPIO_setAsOutputPin(LED, RED);
    GPIO_setAsOutputPin(LED, BLUE);
    GPIO_setOutputLowOnPin(LED, RED);
    GPIO_setOutputLowOnPin(LED, BLUE);

    // sensors - just for now a switch to change status
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT1);



    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConf200);
    Timer_A_configureUpMode(TIMER_A1_BASE, &upModeConf350);
    //Enabling interrupts and going to sleep
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_TA0_N);
    Interrupt_enableInterrupt(INT_TA1_N);

    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    // Enabling MASTER interrupts
    Interrupt_enableMaster();

    // Starting timers
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

}
void sfm_running(){

}
void sfm_danger(){

}
void sfm_emergency(){

}
