#include <stdint.h>
#include <stdio.h>
#include "msp.h"
#include "driverlib/MSP432P4xx/driverlib.h"
#include "_libraries/config_init.h"

/*
 * Maglie Francesco
 * Recruiting project for E-Agle
 * Fall 2022
 */

State_t current_state;
Danger_t danger_status;

float system_voltage;
float sensor_value_lux;


void main(void)
{
    WDT_A_holdTimer(); //stop watch dog timer
    current_state = INIT;
    system_init();
    int bw = 0;
    system_voltage = 0;
    sensor_value_lux = 0;
    current_state = RUNNING;
    danger_status = NONE;
    while(1){
        if(current_state == EMERGENCY){
            printf("Board in waiting state - please press the emergency button\n");
            for(bw = 0; bw < 0x1FFFF; bw++); // implementato con busy waiting
        }else{
            __sleep();
        }
        /*
         * when an interrupt handler in terminated, the while-loop is executed from the
         * beginning and the status is checked
         */
    }

}

/*
 * INTERRUPT HANDLERS
 * for the sake of simplicity, handler functions are imlemented here
 */

void PORT1_IRQHandler(void){
    /*
     * this interrupt handler manages the emergency status
     */
    if(current_state != EMERGENCY){
         /* entering EMERGENCY state -> all handlers are disabled */
        Interrupt_disableInterrupt(INT_TA0_N);
        Interrupt_disableInterrupt(INT_TA1_N);
        Interrupt_disableInterrupt(INT_ADC14);
        current_state = EMERGENCY;
    } else{
        /* exiting EMERGENCY state -> all handlers are enabled */
        Interrupt_enableInterrupt(INT_TA0_N);
        Interrupt_enableInterrupt(INT_TA1_N);
        Interrupt_enableInterrupt(INT_ADC14);
        /*
         * when exiting, the status is set to RUNNING to catch variations
         * (which the system cannot check when in this status) and to update the system
         * as a consequence
         */
        if(danger_status != NONE){
            GPIO_setOutputLowOnPin(LED, RED);
            GPIO_setOutputLowOnPin(LED, BLUE);
        }
        current_state = RUNNING;
//        /*
//         * With this implementation, the status in which the system was before the emergency is restored
//         * (in this case, it is assumed that the system cannot change during EMERGENCY status)
//         */
//        if(danger_status == NONE){
//            /*
//             * vuol dire che quando siamo entrati in EMERGENCY, eravamo in RUNNING status
//             * e quindi lo ripristiniamo
//             */
//            current_state = RUNNING;
//        }else{
//
//            /*
//             * vuol dire che quando siamo entrati in EMERGENCY, eravamo in DANGER status
//             * e quindi lo ripristiniamo
//             */
//            current_state = DANGER;
//        }
    }
    /* reset flag */
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
}

void ADC14_IRQHandler(void)
{
    /*
     * handler related to ADC
     * when a single conversion in completed, the relative
     * interrupt is triggered
     */
    uint64_t status;
    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

}

void TA0_N_IRQHandler(void)
{
    /*
     * handler related to SYSTEM_VOLTAGE checks
     * result of the conversion is stored, then it is evaluated to
     * set the system status properly
     */
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM0);
    system_voltage = (curADCResult * 3.3) / 16384; //2^14
    printf("Value SYSTEM VOLTAGE: %.2f\n", system_voltage);
    switch(current_state){
        case DANGER:
            /*
             * if conditions are satisfied, the RUNNING state is set
             */
            if ((danger_status == OVERVOLTAGE)&&(system_voltage <= OVERVOLTAGE_VALUE))
            {
                GPIO_setOutputLowOnPin(LED, RED);
                danger_status = NONE;
                current_state = RUNNING;
            }else if ((danger_status == UNDERVOLTAGE)&&(system_voltage >= UNDERVOLTAGE_VALUE))
            {
                GPIO_setOutputLowOnPin(LED, BLUE);
                danger_status = NONE;
                current_state = RUNNING;
            }
            break;
        default:
            /*
             * if conditions are satisfied, the RUNNING state is set
             * the associated parameters (danger_status) is also set
             */
            if (system_voltage > OVERVOLTAGE_VALUE)
            {
                GPIO_setOutputHighOnPin(LED, RED);
                danger_status = OVERVOLTAGE;
                current_state = DANGER;
            } else if(system_voltage < UNDERVOLTAGE_VALUE)
            {
                GPIO_setOutputHighOnPin(LED, BLUE);
                danger_status = UNDERVOLTAGE;
                current_state = DANGER;
            }
            break;
    }
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
}

void TA1_N_IRQHandler(void)
{
    /*
     * handler related to SYSTEM_VOLTAGE checks
     * conversion voltage - resistance - luminosity is also performed
     */
    uint16_t curADCResult;
    float sensor_voltage; //voltage provided by ADC
    float res; //conversion voltage to resistance
    curADCResult = ADC14_getResult(ADC_MEM1);
    sensor_voltage = (curADCResult * VOLTAGE_ANALOG_REF) / 16384; //2^14
    //printf("Value PHOTORESISTOR: %.2f\n", sensor_voltage);
    res = (RESISTOR * sensor_voltage) / (VOLTAGE_ANALOG_REF - sensor_voltage);
    //sensor_value_lux = 500/(res/1000);
    sensor_value_lux = 500000/res; //resistance voltage to luminosity
    printf("Value PHOTORESISTOR in LUMEX: %.2f\n", sensor_value_lux);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
}
