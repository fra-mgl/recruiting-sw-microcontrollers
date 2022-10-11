/*
 * config_init.h
 * This file provides all constant values for the system, the definition of all states, and the init function
 */

#ifndef _CONFIG_INIT_H_
#define _CONFIG_INIT_H_

#define LED 2 // LEDs on PORT2
#define RED 0x0001 // Red LED on PIN0
#define BLUE 0x0004 // Blue LED on PIN2
#define VOLT_SYS_PORT 5
#define VOLT_SYS_PIN 0x0020 // P5.5
#define SENSOR_PORT 5
#define SENSOR_PIN 0x0010 // P5.4
#define OVERVOLTAGE_VALUE 2.7
#define UNDERVOLTAGE_VALUE 1.8
#define VOLTAGE_ANALOG_REF 3.3
#define RESISTOR 10000 // 10kOhm resistor

typedef enum {INIT, RUNNING, DANGER, EMERGENCY} State_t;
typedef enum {NONE, OVERVOLTAGE, UNDERVOLTAGE} Danger_t;

void system_init();

#endif /* _CONFIG_INIT_H_ */
