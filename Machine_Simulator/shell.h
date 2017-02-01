/*
	Carl Lindquist
	Dec 7, 2016

	Interface for a shell to run commands. Shell should be kept separate from
	the commands being run. Include new functions into shell.c

	Backspace currently set to '_'
*/



#ifndef SHELL_H
#define SHELL_H

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]	Call constantly with new bytes from stdin to use the shell. 
		Print takes a string and prints it using a custom font.
		The exit command can be called to return to main.

		Valid commands:
    		print [string]
    		exit
	    	hello

[arg1] A byte to be buffered and/or executed when analyzed.
	
[ret]	0 for exit command, 1 otherwise.
*/
int shell(char c);


#endif //SHELL_H