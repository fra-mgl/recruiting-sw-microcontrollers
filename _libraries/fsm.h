/*
 * fsm.h
 *
 * 3 status are defined:
 * RUNNING -> reading sensors, manipulates data and print them
 * DANGER -> undervoltage or overvoltage detected, still reading sensors but toggle led to alert about malfunction
 * EMERGENCY -> it switch is pressed, just show on screen emergency status
 *
 */

#ifndef _FSM_H_
#define _FSM_H_

#define LED 2 // LEDs are configured on PORT2
#define RED 0x0001 // Red LED is on PIN0
#define BLUE 0x0002 // Blue LED is on PIN1

typedef enum {INIT, RUNNING, DANGER, EMERGENCY} State_t;

typedef struct{
    State_t status;
    void (*state_function)(void);
} Machine_t;

void sfm_init();
void sfm_running();
void sfm_danger();
void sfm_emergency();

#endif /* _FSM_H_ */
