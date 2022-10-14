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

//float system_voltage;
//float sensor_value_lux;
int time = 0;
//float sensor_first;
//float sensor_second;
//
//float voltage_first;
//float voltage_second;

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
//    system_voltage = 0;
//    sensor_value_lux = 0;
    current_state = RUNNING;
    danger_status = NONE;
//    sensor_second = SysTick_getValue();
//    voltage_second = SysTick_getValue();
    serial_print("READY");
    while(1){
        //printf("%d sec\n",time);
        if(current_state == EMERGENCY){
            //printf("Board in waiting state - please press the emergency button\n");
            for(bw = 0; bw < 0x5FFFF; bw++); // implementato con busy waiting
            serial_print("Board in waiting state - please press the emergency button");
        }else{
            __sleep();
            //for(bw = 0; bw < 0xFFFF; bw++);
            // print status
            char s[MAX_CHAR];
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
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    //uint32_t my_time = SysTick_getValue();
    system_voltage.end = SysTick_getValue();
    system_voltage.delta_time = (system_voltage.start - system_voltage.end) / FREQ_F;
    system_voltage.start = SysTick_getValue();
    //char s[MAX_CHAR];
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM0);
    system_voltage.value = (curADCResult * 3.3) / 16384; //2^14


    //printf("[%d] Value SYSTEM VOLTAGE: %.2f\n", time,  system_voltage);
//    voltage_first = voltage_second;
//    voltage_second = my_time;
//    float delta = (voltage_first - sensor_second) / 12000000.0;
//    sprintf(s,"[%d] Value SYSTEM VOLTAGE: %.2f [%f]",my_time, system_voltage, delta);
    //sprintf(s,"[%d] Value SYSTEM VOLTAGE: %.2f",my_time, system_voltage);
    //serial_print(s);
//    switch(current_state){
//        case DANGER:
//            /*
//             * if conditions are satisfied, the RUNNING state is set
//             */
//            if ((danger_status == OVERVOLTAGE)&&(system_voltage <= OVERVOLTAGE_VALUE))
//            {
//                GPIO_setOutputLowOnPin(LED, RED);
//                danger_status = NONE;
//                current_state = RUNNING;
//            }else if ((danger_status == UNDERVOLTAGE)&&(system_voltage >= UNDERVOLTAGE_VALUE))
//            {
//                GPIO_setOutputLowOnPin(LED, BLUE);
//                danger_status = NONE;
//                current_state = RUNNING;
//            }
//            break;
//        default:
//            /*
//             * if conditions are satisfied, the RUNNING state is set
//             * the associated parameters (danger_status) is also set
//             */
//            if (system_voltage > OVERVOLTAGE_VALUE)
//            {
//                GPIO_setOutputHighOnPin(LED, RED);
//                danger_status = OVERVOLTAGE;
//                current_state = DANGER;
//            } else if(system_voltage < UNDERVOLTAGE_VALUE)
//            {
//                GPIO_setOutputHighOnPin(LED, BLUE);
//                danger_status = UNDERVOLTAGE;
//                current_state = DANGER;
//            }
//            break;
//    }

}

void TA1_N_IRQHandler(void)
{
    /*
     * handler related to SENSOR checks
     * conversion voltage - resistance - luminosity is also performed
     */
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    sensor.end = SysTick_getValue();
    sensor.delta_time = (sensor.start - sensor.end) / FREQ_F;
    sensor.start = SysTick_getValue();



    uint16_t curADCResult = ADC14_getResult(ADC_MEM1);
    float sensor_voltage = (curADCResult * VOLTAGE_ANALOG_REF) / 16384; //2^14
    float res = (RESISTOR * sensor_voltage) / (VOLTAGE_ANALOG_REF - sensor_voltage);
    sensor.value = 500000/res; //resistance voltage to luminosity










//    uint32_t my_time = SysTick_getValue();
//    char s[MAX_CHAR];
//    uint16_t curADCResult;
//    float sensor_voltage; //voltage provided by ADC
//    float res; //conversion voltage to resistance
//    curADCResult = ADC14_getResult(ADC_MEM1);
//    sensor_voltage = (curADCResult * VOLTAGE_ANALOG_REF) / 16384; //2^14
    //printf("Value PHOTORESISTOR: %.2f\n", sensor_voltage);
//    res = (RESISTOR * sensor_voltage) / (VOLTAGE_ANALOG_REF - sensor_voltage);
    //sensor_value_lux = 500/(res/1000);
//    sensor_value_lux = 500000/res; //resistance voltage to luminosity
    //printf("Value PHOTORESISTOR in LUMEX: %.2f\n", sensor_value_lux);
    //previous_sensor = my_time;
    //float delta = (previous_sensor - my_time) / 12000000.0;
//    sensor_first = sensor_second;
//    sensor_second = my_time;
//    float delta = (sensor_first - sensor_second) / 12000000.0;
//    sprintf(s,"[%d] Sensor value: %.2f [%f]",my_time, sensor_value_lux, delta);
    //sprintf(s,"[%d] Sensor value: %.2f",my_time, sensor_value_lux);
    //serial_print(s);


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

