/*
	Carl Lindquist
	Dec 7, 2016

	Source for a water level simulator shell
*/


#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "machine.h"


#define SHELL_BUFFER_SIZE 64
#define COMMAND_LENGTH SHELL_BUFFER_SIZE/2
#define ARGUMENT_LENGTH SHELL_BUFFER_SIZE/2


char shellBuffer[SHELL_BUFFER_SIZE];
int buffCount = 0;
int exitFlag = 0;



//––––––  Private Declarations  ––––––//
void runCommand(char command[]);
int determineCommand(char command[]);
void clearBuffer(void);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

int shell(char c) {
	if(c == '_') { //Special backspace key for pros
		shellBuffer[--buffCount] = '\0';
		printf("\n\r%s", shellBuffer);
	} else {
		shellBuffer[buffCount++] = c;
	}

	if(buffCount == SHELL_BUFFER_SIZE || c == '\n' || c == '\r') { //command entered
		printf("\n\r");
		shellBuffer[buffCount - 1] = '\0';
		char command[COMMAND_LENGTH] = {};
		if(sscanf(shellBuffer, "%s", command)) { //If a string was found in the buffer
			runCommand(command);
		}
		printf("\n\r%s", SHELL_PROMPT);
		clearBuffer();
	}
	return exitFlag? 0 : 1;
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

enum commands {
	PRINT = 1,
	RUN,
	TOGGLE_POWER,
	RESET,
	HELLO,
	EXIT,
};

/*
[desc]	Runs a set of commands depending on the command given as input.

[command] A string to be analyzed and executed if appropriate.
*/
void runCommand(char command[]) {

	char argument[ARGUMENT_LENGTH] = {};
	int cycles;
	float secondsPerCycle;
	int scans;
	switch (determineCommand(command)) {
		case PRINT:
			sscanf(shellBuffer, "%*s %[^\n]", argument);
			printf("%s\n\r", argument);
			break;

		case RUN:
			scans = sscanf(shellBuffer, "%*s %d %f %s", &cycles, &secondsPerCycle, argument);
			if (scans == 3) {
				runMachine(cycles, secondsPerCycle, 0); //Third arg disables graphics
			} else if (scans == 2) {
				runMachine(cycles, secondsPerCycle, 1);
			} else if (scans == 1) {
				runMachine(cycles, 0.001, 1);
			} else {
				runMachine(200, 0.01, 1); //default run
			}
			break;

		case TOGGLE_POWER:
			printf("\tInfinite power toggled: %s\n\r", togglePower()? "On" : "Off");
			break;

		case RESET:
			machineConfigInit();
			printf("\tMachine Reset\n\r");
			break;

		case HELLO:
			printf("\tHey there :)\n\r");
			break;

		case EXIT:
			exitFlag = 1;
			break;

		default:
			break;
	}
}


/*
[desc]	Determines which command string was passed in as input.

[command] A command string to be compared.

[ret]	Returns values from the "commands" enum above, or 0 if not a command.
*/
int determineCommand(char command[]) {
	if(!strcmp(command, "print")) {
		return PRINT;
	} else if(!strcmp(command, "run")) {
		return RUN;
	} else if(!strcmp(command, "togglePower")) {
		return TOGGLE_POWER;
	} else if(!strcmp(command, "reset")) {
		return RESET;
	} else if(!strcmp(command, "hello")) {
		return HELLO;
	} else if(!strcmp(command, "exit")) {
		return EXIT;
	}
	return 0;
}


/*
[desc]	Empties the shell buffer and resets buffCount.
*/
void clearBuffer(void) {
	int i;
	for(i=0; i < SHELL_BUFFER_SIZE; i++) {
		shellBuffer[i] = '\0';
	}
	buffCount = 0;
}


