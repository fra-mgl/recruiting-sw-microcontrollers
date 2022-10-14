/*
 * config_init.c
 */
#include "_libraries/config_init.h"
#include "driverlib/MSP432P4xx/driverlib.h"

/*
 * Timers parameters
 */

const Timer_A_UpModeConfig upModeConf200 = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_48,
    50000, // 3*10^6 * 200 *10^-3 / 10
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

const Timer_A_UpModeConfig upModeConf350 = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_64,
    65535, //3*10^6 * 350 *10^-3 / 5  20 -> 52.500
    TIMER_A_TAIE_INTERRUPT_ENABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

//const Timer_A_UpModeConfig upModeConf1 = {
//    TIMER_A_CLOCKSOURCE_SMCLK,
//    TIMER_A_CLOCKSOURCE_DIVIDER_48,
//    62500, //3*10^6 * 350 *10^-3 / 5  20 -> 52.500
//    TIMER_A_TAIE_INTERRUPT_ENABLE,
//    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
//    TIMER_A_DO_CLEAR
//};

const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};


void system_init(){

    /*
     * Activating UART Serial communication
     */
    /* Selecting P1.2 and P1.3 in UART mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /*
     * Setting DCO to 12MHz
     * This modifies MCLK and SMCLK frequency and set it to 12MHz
     */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A0_BASE);

    /* Enabling interrupts */
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);

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

    // sensors
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


    SysTick_enableModule();
    SysTick_setPeriod(12000000);
//    SysTick_enableInterrupt();


    Timer_A_configureUpMode(TIMER_A0_BASE, &upModeConf350); // for system voltage
    Timer_A_configureUpMode(TIMER_A1_BASE, &upModeConf200); // for sensor
    //Timer_A_configureUpMode(TIMER_A2_BASE, &upModeConf1); // for timing

    /*
     * Starting...
     */

    //Enabling interrupts and going to sleep
    //Interrupt_enableSleepOnIsrExit();
//    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableInterrupt(INT_TA0_N);
    Interrupt_enableInterrupt(INT_TA1_N);
   // Interrupt_enableInterrupt(INT_TA2_N);

    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    //Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    // Enabling MASTER interrupts
    Interrupt_enableMaster();

    // Starting
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    //Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

}

void serial_print(char* string_to_print){
    int i = 0;
    uint8_t c;
        i = 0;
        c = string_to_print[i];
        while(c != '\0'){
            MAP_UART_transmitData(EUSCI_A0_BASE, c);
            ++i;
            c = string_to_print[i];
        }
        MAP_UART_transmitData(EUSCI_A0_BASE, '\n');
        MAP_UART_transmitData(EUSCI_A0_BASE, '\r');

}

