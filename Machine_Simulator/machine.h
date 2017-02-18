/*
    Carl Lindquist
    Jan 30, 2017

	Library to simulate a machine with flowing water moving between tanks.
	Devices move the water from one tank to the next. Currently this is a
	non-abstracted library, meaning you should change the source to modify
	the operation of the defaultMachineInit() and runMachine state-machine.

*/

#ifndef WATER_MACHINE_H
#define WATER_MACHINE_H



/* Defines for choosing which state-machine to run */
//#define MACHINE_LOW_POWER
// #define MACHINE_COMPROMISE_POWER
#define MACHINE_HIGH_POWER

#define MAX_DEVICE_COUNT 10
#define MAX_TANK_COUNT MAX_DEVICE_COUNT

#define TANK_FULL_THRESHOLD 90
#define TANK_LOW_THRESHOLD 5
#define BATTERY_FULL_THRESHOLD 90
#define BATTERY_LOW_THRESHOLD 20 //Percentage of energy at which the machine waits to recharge


#define DEFAULT_BATTERY_SIZE 1000
#define RECHARGE_PER_CYCLE 10
#define CYCLES_PER_DAY 24 // hours/day




//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//


/*
[desc]	Initializes a machine based on the #defines above.
		Options are:
			MACHINE_LOW_POWER
			MACHINE_COMPROMISE_POWER
			MACHINE_HIGH_POWER	   
*/
void machineConfigInit(void);


/*
[desc]	Initializes the default machine. Currently the only machine implementation is
		the default machine. Does not start the machine. To run the program as quickly
		as possible, set timePerCycle = 0 and set graphicsEnable = 0.

[cycles]	Number of machine cycles to execute
[timePerCycle]	Amount of time in seconds per machine cycle.
				Set this to 0 to run the program as quickly as possbile.
			   
*/
void runMachine(int cycles, float timePerCycle, int graphicsEnable);

/*
[desc]	Toggles infiniteEnergy. Used to decide whether or not the mainBattery is drained.

[ret]	The new value of infiniteEnergy, either 1 or 0.
*/
int togglePower(void);


#include "machineConfig.c"

#endif //SHELL_H