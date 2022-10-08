/*
 * interruptHandlers.c
 */

#include "_libraries/interruptHandlers.h"
#include "driverlib/MSP432P4xx/driverlib.h"
#include <stdio.h>

void TA0_N_IRQHandler(void)
{
    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
}
void TA1_N_IRQHandler(void)
{

}

void PORT1_IRQHandler(void)
{
    //Interrupt_disableInterrupt(INT_TA0_N);
    //Interrupt_disableInterrupt(INT_TA1_N);
    if(Interrupt_isEnabled(INT_TA0_N)){
        printf("interrupt DISABLED\n");
        Interrupt_disableInterrupt(INT_TA0_N);
    }else{
        printf("interrupt ENABLED\n");
        Interrupt_enableInterrupt(INT_TA0_N);
    }
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);

}



