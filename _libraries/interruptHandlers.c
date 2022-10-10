///*
// * interruptHandlers.c
// */
//
//#include "_libraries/interruptHandlers.h"
//#include "_libraries/constants.h"
//#include "driverlib/MSP432P4xx/driverlib.h"
//#include <stdio.h>
//#include <stdint.h>
//
//
//
//void TA0_N_IRQHandler(void)
//{
////    uint16_t curADCResult1;
////    float normalizedADCRes1;
//    // System voltage
////    curADCResult1 = ADC14_getResult(ADC_MEM0);
////    normalizedADCRes1 = (curADCResult1 * 3.3) / 16384; //2^14
////    printf("Value SYSTEM VOLTAGE: %.2f\n", normalizedADCRes1);
//    // NOT a great implementation
////    if(normalizedADCRes1 > 2.8){
////        GPIO_setOutputHighOnPin(LED, RED);
////    } else if(normalizedADCRes1 < 1.7){
////        GPIO_setOutputHighOnPin(LED, BLUE);
////    }else{
////        GPIO_setOutputLowOnPin(LED, RED);
////        GPIO_setOutputLowOnPin(LED, BLUE);
////    }
//    printf("INTR TA\n");
//    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
//}
//void TA1_N_IRQHandler(void)
//{
////    uint16_t curADCResult;
////    float normalizedADCRes;
////    // sensor
////    curADCResult = ADC14_getResult(ADC_MEM1);
////    normalizedADCRes = (curADCResult * 3.3) / 16384; //2^14
////    //printf("Value PHOTORESISTOR: %.2f\n", normalizedADCRes);
//    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
//}
//
//void PORT1_IRQHandler(void)
//{
//    //Interrupt_disableInterrupt(INT_TA0_N);
//    //Interrupt_disableInterrupt(INT_TA1_N);
//    if(Interrupt_isEnabled(INT_TA0_N)){
//        printf("interrupt DISABLED\n");
//        Interrupt_disableInterrupt(INT_TA0_N);
//        Interrupt_disableInterrupt(INT_TA1_N);
//        Interrupt_disableInterrupt(INT_ADC14);
////        ADC14_disableConversion();
//    }else{
//        printf("interrupt ENABLED\n");
//        Interrupt_enableInterrupt(INT_TA0_N);
//        Interrupt_enableInterrupt(INT_TA1_N);
//        Interrupt_enableInterrupt(INT_ADC14);
////        ADC14_enableConversion();
////        ADC14_toggleConversionTrigger();
//    }
//    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
//
//
//
//
//
//
//}
//void ADC14_IRQHandler(void)
//{
//    uint64_t status;
//    uint16_t curADCResult;
//    float normalizedADCRes;
//
//    status = ADC14_getEnabledInterruptStatus();
//
//    //status = 3;
//    if (ADC_INT0 & status) // non arriva segnale interrupt su ADC_INT0
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
//
//    //printf("New sample\n");
//    ADC14_clearInterruptFlag(status);
//
//    ADC14_toggleConversionTrigger();
//
//}
//
//
//
//
//
