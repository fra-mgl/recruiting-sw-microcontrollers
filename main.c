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

typedef struct{
    uint32_t start;
    uint32_t end;
    float delta_time;
    float value;
}Sensor_t;

Sensor_t sensor;
Sensor_t system_voltage;

void check_danger();


void main(void)
{
    WDT_A_holdTimer(); //stop watch dog timer
    current_state = INIT;
    system_init();
    int bw = 0;
    char s[MAX_CHAR];
    current_state = RUNNING;
    danger_status = NONE;
    //serial_print("READY");
    while(1){
        if(current_state == EMERGENCY){
            serial_print("Board in waiting state - please press the emergency button");
            for(bw = 0; bw < 0x5FFFF; bw++);
        }else{
            __sleep();
            sprintf(s,"[%f] Value SYSTEM VOLTAGE: %.2f\n\r[%f] Value SENSOR: %.2f", system_voltage.delta_time, system_voltage.value, sensor.delta_time, sensor.value);
            serial_print(s);
            /*
             * voltage_system is checked only if
             * EMERGENCY status has not been set up
             */
            if (current_state != EMERGENCY) check_danger();
        }
        /*
         * when an interrupt handler in terminated, the while-loop is executed from the
         * beginning and the status is checked
         */
    }

}

/*
 * INTERRUPT HANDLERS
 * for the sake of simplicity, handler functions are implemented here
 */

void PORT1_IRQHandler(void){
    /*
     * this interrupt handler manages the emergency status
     */
    if(current_state != EMERGENCY){
         /* entering EMERGENCY state -> all handlers are disabled */
        Interrupt_disableInterrupt(INT_TA0_N);
        Interrupt_disableInterrupt(INT_TA1_N);
        //Interrupt_disableInterrupt(INT_ADC14);
        current_state = EMERGENCY;
    } else{
        /* exiting EMERGENCY state -> all handlers are enabled */
        Interrupt_enableInterrupt(INT_TA0_N);
        Interrupt_enableInterrupt(INT_TA1_N);
        //Interrupt_enableInterrupt(INT_ADC14);
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
    }
    /* reset flag */
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
}

void TA0_N_IRQHandler(void)
{
    /*
     * handler related to SYSTEM_VOLTAGE checks
     * result of the conversion is stored in system_voltage.value
     */
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    system_voltage.end = SysTick_getValue();
    system_voltage.delta_time = (system_voltage.start - system_voltage.end) / FREQ_F;
    system_voltage.start = SysTick_getValue();
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM0);
    system_voltage.value = (curADCResult * 3.3) / 16384; //2^14

}

void TA1_N_IRQHandler(void)
{
    /*
     * handler related to SENSOR checks
     * conversion voltage - resistance - luminosity is also performed
     * the value is stored in sensor.value
     */
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    sensor.end = SysTick_getValue();
    sensor.delta_time = (sensor.start - sensor.end) / FREQ_F;
    sensor.start = SysTick_getValue();



    uint16_t curADCResult = ADC14_getResult(ADC_MEM1);
    float sensor_voltage = (curADCResult * VOLTAGE_ANALOG_REF) / 16384; //2^14
    float res = (RESISTOR * sensor_voltage) / (VOLTAGE_ANALOG_REF - sensor_voltage);
    sensor.value = 500000/res; //resistance voltage to luminosity
}

void check_danger(){
    switch(current_state){
        case DANGER:
            /*
             * if conditions are satisfied, the RUNNING state is set
             */
            if ((danger_status == OVERVOLTAGE)&&(system_voltage.value <= OVERVOLTAGE_VALUE))
            {
                GPIO_setOutputLowOnPin(LED, RED);
                danger_status = NONE;
                current_state = RUNNING;
            }else if ((danger_status == UNDERVOLTAGE)&&(system_voltage.value >= UNDERVOLTAGE_VALUE))
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
            if (system_voltage.value > OVERVOLTAGE_VALUE)
            {
                GPIO_setOutputHighOnPin(LED, RED);
                danger_status = OVERVOLTAGE;
                current_state = DANGER;
            } else if(system_voltage.value < UNDERVOLTAGE_VALUE)
            {
                GPIO_setOutputHighOnPin(LED, BLUE);
                danger_status = UNDERVOLTAGE;
                current_state = DANGER;
            }
            break;
    }
}
