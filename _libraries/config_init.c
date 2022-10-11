/*
 * fsm.c
 */
#include "_libraries/config_init.h"
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


void system_init(){
    /*
     * configure LEDs
     * configure ADC for sensor sampling
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
    // potentiometer and photoresistor
    GPIO_setAsPeripheralModuleFunctionInputPin(VOLT_SYS_PORT, VOLT_SYS_PIN, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(SENSOR_PORT, SENSOR_PIN, GPIO_TERTIARY_MODULE_FUNCTION);

    /*
     * ADC configuration
     */
    /* Setting Flash wait state */

    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
    // configuring memory and input pin
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, false);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A1, false);
    // Configuring the sample/hold time for TBD
    ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
    ADC14_enableInterrupt(ADC_INT0);
    ADC14_enableInterrupt(ADC_INT1);

    /*
     * Timers configuration
     */

    /* Enabling module for Floating Point Operation */
    FPU_enableModule();
    FPU_enableLazyStacking();

    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConf350); // for system voltage
    Timer_A_configureUpMode(TIMER_A1_BASE, &upModeConf200); // for sensor

    /*
     * Starting...
     */

    //Enabling interrupts and going to sleep
    //Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableInterrupt(INT_TA0_N);
    Interrupt_enableInterrupt(INT_TA1_N);

    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    // Enabling MASTER interrupts
    Interrupt_enableMaster();

    // Starting
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

}

