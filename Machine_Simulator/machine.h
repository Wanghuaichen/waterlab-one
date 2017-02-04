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


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]	Initializes the default machine. Currently the only machine implementation is
		the default machine. Does not start the machine.
*/
void defaultMachineInit(void);

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


#endif //SHELL_H