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



void sfm_init();
void sfm_running();
void sfm_danger();
void sfm_emergency();

void check_danger(float voltage);

#endif /* _FSM_H_ */
