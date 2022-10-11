/*
 * config_init.h
 *
 * In questo file di configurazione iniziale sono presenti i parametri costanti associati al sistema
 * Vengono definiti poi gli stati del sistema [State_t] e una utility per gestire lo stato DANGER [Danger_t]
 * Infine viene dichiarata la funzione di configurazione iniziale
 *
 * Sono definiti i 4 stati:
 * INIT -> stato iniziale durante la configurazione
 * RUNNING -> legge i sensori e manipola i dati (in questo caso li stampa a console)
 * DANEGR -> viene segnalato lo stato di UNDERVOLTAGE o OVERVOLTAGE, continua il processo di lettura dei sensori
 * EMERGENCY -> vengono disattivati gli interrupts per la lettura dei sensori in attesa di uscire dalla modalità
 */

#ifndef _CONFIG_INIT_H_
#define _CONFIG_INIT_H_

#define LED 2 // LEDs configurati su PORT2
#define RED 0x0001 // Red LED è sul pin PIN0
#define BLUE 0x0004 // Blue LED è sul pin PIN1
#define VOLT_SYS_PORT 5
#define VOLT_SYS_PIN 0x0020 // P5.5
#define SENSOR_PORT 5
#define SENSOR_PIN 0x0010 // P5.4
#define OVERVOLTAGE_VALUE 2.7
#define UNDERVOLTAGE_VALUE 1.8

typedef enum {INIT, RUNNING, DANGER, EMERGENCY} State_t;
typedef enum {NONE, OVERVOLTAGE, UNDERVOLTAGE} Danger_t;

void system_init();

#endif /* _CONFIG_INIT_H_ */
