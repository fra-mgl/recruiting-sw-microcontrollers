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
float sensor_voltage;


void main(void)
{
    WDT_A_holdTimer(); //stop watch dog timer
    current_state = INIT;
    system_init();
    int bw = 0;
    system_voltage = 0;
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
         * Quando viene eseguito l'handler di un interrupt, il programma
         * riprende l'esecuzione da qui, viene eseguito un nuovo ciclo del loop
         * e se non ci si trova in stato di EMERGENCY, si ritorna in __sleep()
         */
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
        Interrupt_disableInterrupt(INT_ADC14);
        //ADC14_disableConversion();
        current_state = EMERGENCY;
    } else{
        /* sto uscendo da stato EMERGENCY -> riattivo interrupt e ADC */
        printf("interrupt ENABLED\n");
        Interrupt_enableInterrupt(INT_TA0_N);
        Interrupt_enableInterrupt(INT_TA1_N);
        Interrupt_enableInterrupt(INT_ADC14);
        //ADC14_enableConversion();
        /*
         * all'uscita da EMERGENCY, viene imposto il passaggio per lo stato di RUNNING,
         * per poter aggiornare lo stato nel modo corretto nel caso in cui la tensione
         * fosse cambiata mentre il sistema era in EMERGENCY
         */
        if(danger_status != NONE){
            GPIO_setOutputLowOnPin(LED, RED);
            GPIO_setOutputLowOnPin(LED, BLUE);
        }
        current_state = RUNNING;
//        /*
//         * Questa procedura permette il ripristino dello stato precendente alla chiamata di EMERGENCY
//         * quindi si assume che il system voltage non cambi mentre si è in stato EMERGENCY.
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
     * handler dedicato ad ADC
     * semplicemente notifica la fine di un ciclo di conversione
     * la lettura viene eseguita dall'appostito handler del timer
     * che ne scandisce il timing
     */
    uint64_t status;
    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

}

void TA0_N_IRQHandler(void)
{
    /*
     * Handler dedicato alla lettura e al controllo di
     * SYSTEM_VOLTAGE sul pin P5.4
     * in questo contesto viene anche eseguito il controllo
     * e la modifica di stato del sistema
     */
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM0);
    system_voltage = (curADCResult * 3.3) / 16384; //2^14
    printf("Value SYSTEM VOLTAGE: %.2f\n", system_voltage);
    switch(current_state){
        case DANGER:
            /*
             * se soddisfatte le condizioni, usciamo dallo stato DANGER
             * e ripristiniamo lo stato di RUNNING
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
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
}

void TA1_N_IRQHandler(void)
{
    /*
     * Handler dedicato alla lettura del sensore associato
     * al pin P5.4
     */
    uint16_t curADCResult;
    curADCResult = ADC14_getResult(ADC_MEM1);
    sensor_voltage = (curADCResult * 3.3) / 16384; //2^14
    printf("Value PHOTORESISTOR: %.2f\n", sensor_voltage);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
}
