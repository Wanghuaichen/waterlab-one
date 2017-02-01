/*
	Carl Lindquist
	Dec 7, 2016

	Playground for waterlab one testing state machine
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "shell.h"
#include "machine.h"

int main (int argc, char* argv[]) {
	//system("/bin/stty raw echo inlcr");
	system("clear");
	printf("----- Machine -----\n\n\r");

	defaultMachineInit();
	
	while(shell(getchar()));

	//system ("/bin/stty sane");
	return 0;
}



/* EOF */