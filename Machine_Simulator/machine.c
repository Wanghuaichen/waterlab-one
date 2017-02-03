/*
	Carl Lindquist
	Jan 29, 2017

	Library to simulate a machine with flowing water moving between tanks.
	Devices move the water from one tank to the next. Currently this is a
	non-abstracted library, meaning you should change the source to modify
	the operation of the defaultMachineInit() and runMachine state-machine.
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "machine.h"


#define TRUE 1
#define FALSE 0
#define INFINITY 999999

#define TANK_FULL_THRESHOLD 90
#define BATTERY_FULL_THRESHOLD 90
#define BATTERY_LOW_THRESHOLD 20 //Percentage of energy at which the machine waits to recharge
#define DEFAULT_BATTERY_SIZE 1000
#define RECHARGE_PER_CYCLE 10
#define CYCLES_PER_DAY 24 // hours/day
#define HALF_DAY (CYCLES_PER_DAY / 2)

#define TANK_PRINT_WIDTH 4
#define TANK_PRINT_HEIGHT 11
#define MAX_TANK_COUNT 10
#define SPACING 3


//––––––  Private Types  ––––––//
typedef struct Tank {
	int volume;
	int quantity;
	float turbidity;
} Tank;

typedef struct Device {
	int enable;
	int flowRate;
	int consumption;
	float newTurbidity;
	Tank* source;
	Tank* sink;
} Device;

typedef struct Battery {
	int remaining;
	int max;
} Battery;

enum STATES {
	STATE_IDLE,
	STATE_RUN_FILTER_PUMP,
	STATE_RUN_RO_PUMP,
	STATE_RUN_UV,
	STATE_RECHARGE,
} machineState;


//––––––  Private Variables  ––––––//
Device* devices[MAX_TANK_COUNT] = {};
int numDevices;
Tank* tanks[MAX_TANK_COUNT] = {};
int numTanks;
Battery* mainBattery;
int totalCycles;
int daytime;
int waterPurified;


//––––––  Private Declarations  ––––––//
Tank* tank(int volume, int quantity, float turbidity);
Device* device(int enable, int flowRate, int consumption, float newTurbidity, Tank* source, Tank* sink);
Battery* battery(int remaining, int max);
void updateMachine(void);
int moveWater(Tank* source, Tank* sink, int amount, float sourceTurbidity);
void runDevice(Device* device);
void printTanks(Tank* tankArr[MAX_TANK_COUNT]);
void stageTank(char tankStage[][TANK_PRINT_WIDTH], Tank* tank);
void printTurbidities(Tank* tank[MAX_TANK_COUNT]);
void printBattery(Battery* battery);
int deviceAvailable(Device* device);
int isFull(Tank* tank);
void delay(double dly);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void runMachine(int cycles, float timePerCycle, int graphicsEnable) {
	int i;
	while(i++ < cycles) { //Run the machine for cycles 
		mainBattery->remaining += daytime ? RECHARGE_PER_CYCLE : 0; //recharge if its daytime
		
		updateMachine(); // run the state machine
		if(graphicsEnable) {
			system("clear");
			printTanks(tanks);
			printTurbidities(tanks);
			printBattery(mainBattery);
		}
		delay(timePerCycle); //Waste time so the user can watch the tanks change graphically

		totalCycles++;
		if (totalCycles % HALF_DAY == 0) { // invert daytime every half day
			daytime = daytime ? FALSE : TRUE;  
		}
	}
	/* Print the final state even with graphics disabled */
	system("clear");
	printTanks(tanks);
	printTurbidities(tanks);
	printBattery(mainBattery);
	printf("\n----- Done -----\n\r");
	printf("[cycles run] %d\n\r\tSome more stats\n\r" , cycles);
}

/*
[desc]	Initializes a machine described in two storage arrays. Not yet public
		due to the low-modularity of machine implementation.

[tankArr] Array containing Tank objects, size must be MAX_TANK_COUNT
[deviceArr]	Array containing Device objects, size must be MAX_TANK_COUNT	
[batterySize] Size of the battery for the machine		   
*/
void machineInit(Tank* tankArr[MAX_TANK_COUNT], Device* deviceArr[MAX_TANK_COUNT], int batterySize) {
	int i;
	for (i=0; i < MAX_TANK_COUNT; i++) //Copy to global
		tanks[i] = tankArr[i];
	for (i=0; i < MAX_TANK_COUNT; i++)
		devices[i] = deviceArr[i];

	for (i=0; tanks[i] != NULL; i++); //Counts for bookeeping
	numTanks = i;
	for (i=0; devices[i] != NULL; i++);
	numDevices = i;
	machineState = STATE_IDLE;

	mainBattery = battery(batterySize, batterySize);
	totalCycles = 0;
	daytime = FALSE;
	waterPurified = 0;
}


void defaultMachineInit(void) {
	//Initialized outside of array for readable device initialization
	/* tank(volume, quantity, turbidity) */
	Tank* source = tank(INFINITY, INFINITY, 5.0);
	Tank* tank2 = tank(25, 0, 0.0);
	Tank* tank3 = tank(25, 0, 0.0);
	Tank* tank4 = tank(25, 0, 0.0);
	Tank* sink = tank(INFINITY, 0, 0.0);

	Tank* tankArr[MAX_TANK_COUNT] = {};
	tankArr[0] = source;
	tankArr[1] = tank2;
	tankArr[2] = tank3;
	tankArr[3] = tank4;
	tankArr[4] = sink;

	Device* deviceArr[MAX_TANK_COUNT] = {};
	/* device(enable, flowRate, powerconsumption, newTurbidity, source, sink) */
	//Filter pump
	deviceArr[0] = device(FALSE, 15, 10, 3.0, source, tank2);
	//RO pump
	deviceArr[1] = device(FALSE, 2, 10, 0.3, tank2, tank3);
	deviceArr[4] = device(FALSE, 8, 0, -1, tank2, sink); //RO reject water
	//UV disinfect
	deviceArr[2] = device(FALSE, 100, 50, -1, tank3, tank4);
	//Slow drain
	deviceArr[3] = device(FALSE, 50, 0, -1, tank4, sink);
	
	machineInit(tankArr, deviceArr, DEFAULT_BATTERY_SIZE); //Load tanks and devices into global arrays
	printf(""); //Very nasty fix to make the terminal keep up
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]	Constructor for a Tank object. Use devices to move water betwen tanks.
			Limitless source: volume = INFINITY, capacity = INFINITY
			Bottomless sink: volume = INFINITY, capacity = 0

[volume] Int representing the maximum capacity of the tank.
[quantity] Int representing how much water is currently in the tank.
[turbidity] Float representing the tank's turbidity.

[ret]	A pointer to the newly created tank object.	   
*/
Tank* tank(int volume, int quantity, float turbidity) {
    Tank* t = (Tank*) malloc(sizeof(Tank));
    t->turbidity = turbidity;
    t->volume = volume;
    t->quantity = quantity;
	return t;
}

/*
[desc]	Constructor for a Device object. Devices are designed to move water
		between tanks. Only moves water when they are on.

[enable] Decides whether or not the device moves water each cycle.
[flowRate] Water to be moved by the device per cycle if enable = 1.
[consumpion] Power consumed by the device (not yet implemented).
[newTurbidity] Maximum turbidity of water flowing out of this device.
[source] Tank to source from.
[sink] Tank to sink into.

[ret]	A pointer to the newly created device object.	   
*/
Device* device(int enable, int flowRate, int consumption, float newTurbidity, Tank* source, Tank* sink) {
	Device* d = (Device*) malloc(sizeof(Device));
	d->enable = enable;
	d->flowRate = flowRate;
	d->consumption = consumption;
	d->newTurbidity = newTurbidity;
	d->source = source;
	d->sink = sink;
	return d;
}

/*
[desc]	Constructor for a Battery object. Batteries power devices.

[remaining] Int representing the remaining energy in the battery.
[max] Int representing the maximum energy the battery can store.

[ret]	A pointer to the newly created battery object.	   
*/
Battery* battery(int remaining, int max) {
    Battery* b = (Battery*) malloc(sizeof(Battery));
    b->remaining = remaining;
    b->max = max;
	return b;
}


/*
[desc]	This is the system state-machine. Runs in the following way:
			If the battery power is low, recharge until BATTERY_FULL_THRESHOLD
			If a device can run run it, if not try the next one
				Once a device is running, continue until the 'sink tank' is full or out of power
					Return to the idle state once a device can no longer run
*/
void updateMachine(void) {
	int i;
	Device* filterPump = devices[0];
	Device* roPump = devices[1];
	Device* roReject = devices[4];
	Device* uv = devices[2];
	Device* drain = devices[3];

	switch (machineState) {
		case STATE_IDLE:
			if (mainBattery->remaining*100 / mainBattery->max < BATTERY_LOW_THRESHOLD) {
				machineState = STATE_RECHARGE;
			} else if (deviceAvailable(filterPump)) { // Filter pump
				filterPump->enable = TRUE;
				machineState = STATE_RUN_FILTER_PUMP;
			} else if (deviceAvailable(roPump)) { // RO pump
				roPump->enable = TRUE;
				roReject->enable = TRUE;
				machineState = STATE_RUN_RO_PUMP;
			} else if (deviceAvailable(uv)) { // UV 
				uv->enable = TRUE;
				machineState = STATE_RUN_UV;
			} else {
				//Drain the final tank
				drain->enable = TRUE;
				runDevice(drain);
			}
			break;

		case STATE_RUN_FILTER_PUMP:
			if (deviceAvailable(filterPump)) {
				runDevice(filterPump);
			} else {
				filterPump->enable = FALSE;
				machineState = STATE_IDLE;
			}
			break;

		case STATE_RUN_RO_PUMP:
			if (deviceAvailable(roPump)) {
				runDevice(roPump);
				runDevice(roReject);
			} else {
				roPump->enable = FALSE;
				roReject->enable = FALSE;
				machineState = STATE_IDLE;
			}
			break;

		case STATE_RUN_UV:
			if (deviceAvailable(uv)) {
				waterPurified += runDevice(uv);
			} else {
				uv->enable = FALSE;
				machineState = STATE_IDLE;
			}
			break;

		case STATE_RECHARGE:
			if (mainBattery->remaining*100 / mainBattery->max >= BATTERY_FULL_THRESHOLD) {
				machineState = STATE_IDLE;
			}
			break;
	};
}

/*
[desc]	Removes energy from the global battery if consumption is less than the energy
		remining in the battery.

[consumption] Amount of energy to remove from the battery.

[ret]	Returns 1 if the battery has enough energy remaining, 0 otherwise.
*/
int drainBattery(int consumption) {
	if (mainBattery->remaining < consumption) {
		return 0;
	}
	mainBattery->remaining = mainBattery->remaining - consumption;
	return 1;
}

/*
[desc]	Moves water from one tank into another. Only moves water if the
		water is availble in the 'source tank'. Will not overfill the 'sink tank'.
			Water movement: 'source tank' -> 'sink tank'

[source] Tank to source water from.
[sink] Tank to sink water into.
[amount] Amount of water to move.
[sourceTurbidity] Turbidity of the incoming water.

[ret]	Water moved between tanks.
*/
int moveWater(Tank* source, Tank* sink, int amount, float sourceTurbidity) {
	/* set new turbidity for sink using weighted average */
	sink->turbidity = ( (sourceTurbidity*(float)amount) + (float)(sink->turbidity*sink->quantity) ) /
		(float)(sink->quantity + amount);

	if (amount > source->quantity) { //Source doesnt have enough
		amount = source->quantity;
	}
	if (amount + sink->quantity > sink->volume) { //Sink is too full
		amount = sink->volume - sink->quantity; 
	}

	source->quantity -= amount;
	sink->quantity += amount;
	return amount;

	// /* increase sink quantity */
	// if (sink->volume != INFINITY) { 
	// 	if (source->quantity < amount) {
	// 		sink->quantity += source->quantity; //source doesn't have enough
	// 	} else {
	// 		sink->quantity += amount; //source has enough
	// 	}

	// 	if (sink->quantity > sink->volume) {
	// 		sink->quantity = sink->volume; //Cap volume at full tank
	// 	}
	// }

	// /* decrease source tank */
	// if (source->quantity != INFINITY && source->quantity > 0) {
	// 	source->quantity -= amount;
	// 	if (source->quantity < 0) {
	// 		source->quantity = 0; //Cap volume at empty tank
	// 	}
	// }
	// return isFull(sink)? FALSE: TRUE;
}

/*
[desc]	Calls drainBattery() and moveWater() on the device specified. Calculates the incoming
		water turbidity from the device specified. Does no checking on whether water should 
		actually be moved.

[device] A device to run.

[ret]	Returns the amount of water moved.
*/
int runDevice(Device* device) {
	float sourceTurbidity;
	if (device->newTurbidity < device->source->turbidity && device->newTurbidity >= 0.0) {
		sourceTurbidity = device->newTurbidity;
	} else {
		sourceTurbidity = device->source->turbidity;
	}
	drainBattery(device->consumption);
	return moveWater(device->source, device->sink, device->flowRate, sourceTurbidity);
}

/*
[desc]	Determines whether or not a tank's quantity is above TANK_FULL_TRESHOLD% full.

[tank] A tank to examine.

[ret]	1 if the tank is full, 0 otherwise.
*/
int isFull(Tank* tank) {
	int vol = tank->volume;
	int qty = tank->quantity;
	return (TANK_FULL_THRESHOLD <= (float)qty / (float)vol * 100)? TRUE: FALSE;
}

/*
[desc]	Determines whether or not a device can be run for the next cycle based
		on power to be consumed, and the fullness of the source tank.

[tank] A device to examine.

[ret]	1 if the device can run, 0 otherwise.
*/
int deviceAvailable(Device* device) {
	if (!isFull(device->sink) && device->consumption <= mainBattery->remaining) {
		return 1;
	} else {
		return 0;
	}
}

/*
[desc]	Loops for dly amount of time.

[dly] Delay in seconds.
*/
void delay(double dly){
    clock_t ticks1, ticks2;

	ticks1=clock();
	ticks2=ticks1;
	while(((double)ticks2/CLOCKS_PER_SEC - (double)ticks1/CLOCKS_PER_SEC) < dly)
		ticks2=clock();
}


//––––––––––––––––––––––––––––––  Print Functions  ––––––––––––––––––––––––––––––//

/*
[desc]	Prints a visual representation of the fullness of all tanks in an array.
		Loads tank stages into a 2D array using the stageTank() helper function.

[tanks] An array of tanks to print. Must be MAX_TANK_COUNT in length.
*/
void printTanks(Tank* tanks[MAX_TANK_COUNT]) {
	char lineStage[TANK_PRINT_HEIGHT][MAX_TANK_COUNT * (TANK_PRINT_WIDTH + SPACING)] = {};
	char tankStage[TANK_PRINT_HEIGHT][TANK_PRINT_WIDTH] = {};

	int l, i, j, t;
	int tanksPrinted = 0;

	int numTanks;
	for (i=0; tanks[i] != NULL && i < MAX_TANK_COUNT; i++);
	numTanks = i;
	
	//transfer letters to the line stage
	for(l=0, t=0; t < numTanks; t++) {
		stageTank(tankStage, tanks[t]);
		tanksPrinted++;
		for(i=0; i < TANK_PRINT_HEIGHT; i++) {
			for(j=0; j < TANK_PRINT_WIDTH; j++) { //Add tank to linestage
				lineStage[i][j + l] = tankStage[i][j];
			}
			for(; j < TANK_PRINT_WIDTH + SPACING; j++) { //add spacing
				lineStage[i][j + l] = ' ';
			}
		}
		l += TANK_PRINT_WIDTH + SPACING; //Start index of new tank	
	}

	//print entire stage
	printf("\n\r");
	for(i = TANK_PRINT_HEIGHT - 1; i >= 0; i--) {
		for(j = 0; j < numTanks*(TANK_PRINT_WIDTH + SPACING); j++) {
			printf("%c", lineStage[i][j]);
		}
		printf("\n\r");
	}
}

/*
[desc]	Helper function for printTanks(). Print-stages a specific tank into an array.

[tankStage] A 2D array sized specifically for print-staging tanks.
[tank] A tank to stage.
*/
void stageTank(char tankStage[][TANK_PRINT_WIDTH], Tank* tank) {
	int waterLevel = (tank->quantity*10)/tank->volume;

	int i, j;
	for(i=0; i < TANK_PRINT_HEIGHT; i++) {
		for(j=0; j < TANK_PRINT_WIDTH; j++) {
			if ((j == 0 || j == TANK_PRINT_WIDTH - 1) && i)
				tankStage[i][j] = '|';
			else if (!i)
				tankStage[i][j] = '=';
			else if (i <= waterLevel)
				tankStage[i][j] = '-';
			else
				tankStage[i][j] = ' ';
		}
	}
}

/*
[desc]	Prints the turbidities of all tanks in tankArr on one line.

[tanks] An array of tanks to print. Must be MAX_TANK_COUNT in length.
*/
void printTurbidities(Tank* tankArr[MAX_TANK_COUNT]) {
	int i;
	for (i=0; tankArr[i] != NULL && i < MAX_TANK_COUNT; i++) {
		printf("%04.2f   ", tankArr[i]->turbidity);
	}
	printf("\n\r");
}

/*
[desc]	Prints the battery statistics.

[battery] A battery to print stats for.
*/
void printBattery(Battery* battery) {
	printf("\n\rBattery %%: %d\n\r", battery->remaining*100/battery->max);
	printf("Daytime: %s\n\n\r", daytime ? "Yes" : "No");
}

void printStats(void) {
	printf()
}











/*EOF*/