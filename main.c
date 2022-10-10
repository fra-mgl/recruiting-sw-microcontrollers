#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "msp.h"
#include "driverlib/MSP432P4xx/driverlib.h"
#include "_libraries/fsm.h"
#include "_libraries/constants.h"
//#include "_libraries/interruptHandlers.h"

#define NUM_STATES 4;

/*
 * Maglie Francesco
 * Recruiting project for E-Agle
 */


typedef enum {INIT, RUNNING, DANGER, EMERGENCY} State_t;
typedef enum {NONE, OVERVOLTAGE, UNDERVOLTAGE} Danger_t;

typedef struct{
    State_t status;
    void (*state_function)(void);
} Machine_t;

State_t current_state;
Danger_t danger_status;

Machine_t fsm[4] = {
                 {INIT, sfm_init},
                 {RUNNING, sfm_running},
                 {DANGER, sfm_danger},
                 {EMERGENCY, sfm_emergency}
                 /*
                  * in teoria running e danger non hanno funzioni perchè sono gestiti tramite gli interrupt
                  */
};
int i;
float system_voltage;
float sensor_voltage;


//(*(fsm[0].state_function))();

void main(void)
{
    WDT_A_holdTimer();
    current_state = INIT;
    (*(fsm[INIT].state_function))();//sfm_init();
    system_voltage = 0;
    current_state = RUNNING;
    danger_status = NONE;
    while(1){
        printf("MAIN CS: %d\n\n", current_state);
        if(current_state == EMERGENCY){
            //(*(fsm[EMERGENCY].state_function))(); //sfm_emergency
            printf("Board in waiting state - please press the emergency button\n");
            for(i = 0; i < 0x5FFF; i++); // implementato con busy waiting
        }
        //printf("ENTER SLEEEP\n");
        __sleep();
        //printf("EXIT SLEEP\n");
//        else{
//            printf("ENTER SLEEEP\n");
//            __sleep();
//            printf("EXIT SLEEP\n");
//        }
    }

}

/*
 * INTERRUPT HANDLERS
 * Vengono gestiti nel file main.c per comodità e semplicità
 */

void PORT1_IRQHandler(void){
    if(current_state != EMERGENCY){
         /* sto entrando in stato EMERGENCY -> disattivo tutti gli interrupts e ADC */
        printf("interrupt DISABLED\n");
        Interrupt_disableInterrupt(INT_TA0_N);
        Interrupt_disableInterrupt(INT_TA1_N);
        //Interrupt_disableInterrupt(INT_ADC14);
        //ADC14_disableConversion();
        current_state = EMERGENCY;
    } else{
        /* sto uscendo da stato EMERGENCY -> riattivo interrupt e ADC */
        printf("interrupt ENABLED\n");
        Interrupt_enableInterrupt(INT_TA0_N);
        Interrupt_enableInterrupt(INT_TA1_N);
        //Interrupt_enableInterrupt(INT_ADC14);
        //ADC_enableConversion();
        if(danger_status == NONE){
            /*
             * vuol dire che quando siamo entrati in EMERGENCY, eravamo in RUNNING status
             * e quindi lo ripristiniamo
             */
            current_state = RUNNING;
        }else{

            /*
             * vuol dire che quando siamo entrati in EMERGENCY, eravamo in DANGER status
             * e quindi lo ripristiniamo
             * PER ORA, UNICA FALLA RISCONTRATA E' CHE SE VIENE CAMBIATA LA TENSIONE DEL SISTEMA
             * DURANTE STATO DI EMERGENCY, LED NON CAMBIA IN MODO CORRETTO
             */
            current_state = DANGER;
        }
        /*
         * se ci sono led accesi vado in danger, altrimenti in runnning
         */
    }
    /* reset flag */
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
}

void ADC14_IRQHandler(void)
{
    uint64_t status;
//    uint16_t curADCResult;
//    float normalizedADCRes;

    status = ADC14_getEnabledInterruptStatus();

//    //status = 3;
//    if (ADC_INT0 & status) // non arriva segnale interrupt su ADC_INT0 -- ora dovrebbe funzionare perchè configurato in multiple sampling
//    {
//        curADCResult = ADC14_getResult(ADC_MEM0);
//        normalizedADCRes = (curADCResult * 3.3) / 16384; //2^14
//        printf("Value SYSTEM VOLTAGE: %.2f\n", normalizedADCRes);
//    }
//    if (ADC_INT1 & status)
//    {
//        curADCResult = ADC14_getResult(ADC_MEM1);
//        normalizedADCRes = (curADCResult * 3.3) / 16384; //2^14
//        printf("Value PHOTORESISTOR: %.2f\n", normalizedADCRes);
//    }

    //printf("New sample\n");
    ADC14_clearInterruptFlag(status);

    //ADC14_toggleConversionTrigger();
    // va in automatico, controllo con timers la lettura

}

void TA0_N_IRQHandler(void)
{
    //system voltage
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM0);
    system_voltage = (curADCResult * 3.3) / 16384; //2^14
    //printf("Value SYSTEM VOLTAGE: %.2f\n", system_voltage);
    //printf("Check danger\n");

    switch(current_state){
        case DANGER:
            /*
             * se soddisfatte le condizioni, usciamo dallo stato DANGER
             */
            if ((danger_status == OVERVOLTAGE)&&(system_voltage <= OVERVOLTAGE_VALUE))
            {
                GPIO_setOutputLowOnPin(LED, RED);
                danger_status = NONE;
                current_state = RUNNING;
            }else if((danger_status == UNDERVOLTAGE)&&(system_voltage >= UNDERVOLTAGE_VALUE))
            {
                GPIO_setOutputLowOnPin(LED, BLUE);
                danger_status = NONE;
                current_state = RUNNING;
            }
            break;
        default:
            /*
             * se soddisfatte le condizioni, entriamo nello stato DANGER
             * e impostiamo il relativo parametro
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




//    switch(current_state){
//        case DANGER:
//            if (system_voltage <= OVERVOLTAGE)
//            {
//                GPIO_setOutputLowOnPin(LED, RED);
//                current_state = RUNNING;
//            }else if(system_voltage >= UNDERVOLTAGE){
//                GPIO_setOutputLowOnPin(LED, BLUE);
//                current_state = RUNNING;
//            }
//            break;
//        default:
//            if (system_voltage > OVERVOLTAGE)
//            {
//                GPIO_setOutputHighOnPin(LED, RED);
//                overvoltage = true;
//                current_state = DANGER;
//            } else if(system_voltage < UNDERVOLTAGE){
//                GPIO_setOutputHighOnPin(LED, BLUE);
//                current_state = DANGER;
//            }
//            break;
//    }
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
}
void TA1_N_IRQHandler(void)
{
    // sensor
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM1);
    sensor_voltage = (curADCResult * 3.3) / 16384; //2^14
    //printf("Value PHOTORESISTOR: %.2f\n", sensor_voltage);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
}
