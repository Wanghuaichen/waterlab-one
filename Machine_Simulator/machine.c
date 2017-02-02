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

#define TANK_FULL_THRESHOLD 99.5

#define TANK_PRINT_WIDTH 4
#define TANK_PRINT_HEIGHT 11
#define MAX_TANK_COUNT 10
#define SPACING 3


//––––––  Private Types  ––––––//
typedef struct Tank{
	int volume;
	int quantity;
	float turbidity;
} Tank;

typedef struct Device{
	int enable;
	int flowRate;
	int consumption;
	float newTurbidity;
	Tank* source;
	Tank* sink;
} Device;


//––––––  Private Variables  ––––––//
Device* devices[MAX_TANK_COUNT] = {};
int numDevices;
Tank* tanks[MAX_TANK_COUNT] = {};
int numTanks;


//––––––  Private Declarations  ––––––//
Tank* tank(int volume, int quantity, float turbidity);
Device* device(int enable, int flowRate, int consumption, float newTurbidity, Tank* source, Tank* sink);
void updateMachine(void);
int moveWater(Tank* source, Tank* sink, int amount, float sourceTurbidity);
void updateDevice(Device* device);
void printTanks(Tank* tankArr[MAX_TANK_COUNT]);
void stageTank(char tankStage[][TANK_PRINT_WIDTH], Tank* tank);
void printTurbidities(Tank* tank[MAX_TANK_COUNT]);
int isFull(Tank* tank);
void delay(double dly);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void runMachine(int cycles, float timePerCycle, int graphicsEnable) {
	int i;
	while(i++ < cycles) { //Run the machine for cycles 
		updateMachine(); //Move water around and update device statuses
		if(graphicsEnable) {
			system("clear");
			printTanks(tanks);
			printTurbidities(tanks);
		}
		delay(timePerCycle); //Waste time so the user can watch the tanks change
	}
	printf("\n----- Done -----\n\r");
	printf("[cycles run] %d\n\r\tSome more stats\n\r" , cycles);
}

/*
[desc]	Initializes a machine described in two storage arrays. Not yet public
		due to the low-modularity of machine implementation.

[tankArr] Array containing Tank objects, size must be MAX_TANK_COUNT
[deviceArr]	Array containing Device objects, size must be MAX_TANK_COUNT			   
*/
void machineInit(Tank* tankArr[MAX_TANK_COUNT], Device* deviceArr[MAX_TANK_COUNT]) {
	int i;
	for (i=0; i < MAX_TANK_COUNT; i++) //Copy to global
		tanks[i] = tankArr[i];
	for (i=0; i < MAX_TANK_COUNT; i++)
		devices[i] = deviceArr[i];

	for (i=0; tanks[i] != NULL; i++); //Counts for bookeeping
	numTanks = i;
	for (i=0; devices[i] != NULL; i++);
	numDevices = i;
}


void defaultMachineInit(void) {
	//Initialized outside of array for readable device initialization
	/* tank(volume, quantity, turbidity) */
	Tank* source = tank(INFINITY, INFINITY, 5.0);
	Tank* tank2 = tank(100, 0, 0.0);
	Tank* tank3 = tank(100, 0, 0.0);
	Tank* tank4 = tank(100, 0, 0.0);
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
	deviceArr[0] = device(FALSE, 14, 10, 3.0, source, tank2);
	//RO pump
	deviceArr[1] = device(FALSE, 5, 9, 0.5, tank2, tank3);
	//UV disinfect
	deviceArr[2] = device(FALSE, 20, 10, -1, tank3, tank4);
	//Slow drain
	deviceArr[3] = device(FALSE, 3, 15, -1, tank4, sink);

	
	machineInit(tankArr, deviceArr); //Load tanks and devices into global arrays
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
[desc]	For all initialized devices, calls moveWater() and updateDevice(), see
		below for details. Has the effect of 'running the machine' by moving water 
		from tank to tank depending on wheter devices are on or off. Also
		enables/disables machines based on the functionality of updateDevice().
*/
void updateMachine(void) {
	int i;
	float sourceTurbidity;
	for (i=0; i < numDevices; i++) {
		if (devices[i]->enable) {
			if (devices[i]->newTurbidity < devices[i]->source->turbidity && devices[i]->newTurbidity >= 0.0)
				sourceTurbidity = devices[i]->newTurbidity;
			else
				sourceTurbidity = devices[i]->source->turbidity;
			// sourceTurbidity =  ? devices[i]->newTurbidity : devices[i]->source->turbidity;
			moveWater(devices[i]->source, devices[i]->sink, devices[i]->flowRate, sourceTurbidity);
		}
		updateDevice(devices[i]);
	}
}

/*
[desc]	Moves water from one tank into another. Only moves water if the
		water is availble in the 'source tank'. Will not overfill the 'sink tank'.
			Water movement: 'source tank' -> 'sink tank'

[source] Tank to source water from.
[sink] Tank to sink water into.
[amount] Amount of water to move.
[sourceTurbidity] Turbidity of the incoming water.

[ret]	0 if 'sink tank' was filled, 1 otherwise.
*/
int moveWater(Tank* source, Tank* sink, int amount, float sourceTurbidity) {
	/* increase sink quantity */
	if (sink->volume != INFINITY) { 
		if (source->quantity < amount)
			sink->quantity += source->quantity; //source doesn't have enough
		else 
			sink->quantity += amount; //source has enough
		if (sink->quantity > sink->volume)
			sink->quantity = sink->volume; //Cap volume at full tank
	}

	/* decrease source tank */
	if (source->quantity != INFINITY && source->quantity > 0) {
		source->quantity -= amount;
		if (source->quantity < 0)
			source->quantity = 0; //Cap volume at empty tank
	}

	/* set new turbidity for sink using weighted average */
	sink->turbidity = ( (sourceTurbidity*(float)amount) + (sink->turbidity*sink->quantity) ) /
		(sink->quantity + (float)amount);

	return isFull(sink)? FALSE: TRUE;
}

/*
[desc]	Enables/disables a device based on the fullness of the sink tank.

[device] Device to update.
*/
void updateDevice(Device* device) {
	if (device->sink->quantity == device->sink->volume)
		device->enable = FALSE;
	else if (device->sink->quantity <= device->sink->volume/2)
		device->enable = TRUE;
}

/*
[desc]	Determines whether or not a tank's quantity is above TANK_FULL_TRESHOLD % full.

[tank] A tank to examine.

[ret]	1 if the tank is full, 0 otherwise.
*/
int isFull(Tank* tank) {
	int vol = tank->volume;
	int qty = tank->quantity;
	return (TANK_FULL_THRESHOLD >= (float)qty / (float)vol * 100)? TRUE: FALSE;
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

	//print stage
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

void printTurbidities(Tank* tankArr[MAX_TANK_COUNT]) {
	// int numTanks;
	int i;
	for (i=0; tankArr[i] != NULL && i < MAX_TANK_COUNT; i++) {
		printf("%04.2f   ", tankArr[i]->turbidity);
	}
	// numTanks = i;
	printf("\n\r");
}







/*EOF*/