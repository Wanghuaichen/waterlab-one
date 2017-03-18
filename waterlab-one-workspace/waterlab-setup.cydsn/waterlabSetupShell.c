/* 
    Carl Lindquist
    Mar 17, 2017
    
    A shell for communication between the PSOC 5LP
    via USBUART for setting up water lab components. 
*/
    
#include "waterlabSetupShell.h"
#include "usbProtocol.h"
#include "ezoProtocol.h"

#include <stdio.h>
#include <string.h>

#define SHELL_BUFFER_SIZE 64
#define COMMAND_LENGTH 32
#define ARGUMENT_LENGTH 32
#define EXIT_SHELL 0


char buffer[SHELL_BUFFER_SIZE];


//–––––– Private Declarations ––––––//

uint8 shellProcessByte(uint8 byte);
uint8 runCommand(void);
uint8 determineCommand(char command[]);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void shellRun(void) {
    usbSendString("<--> ");
    while (shellProcessByte(usbGetByte()) != EXIT_SHELL);    
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Input bytes are buffered until '\r' received or buffer is full. Upon execute,
		runCommand() checks buffer for a valid command.
        
[byte]	Bytes to be buffered, '\r' executes buffer.
    
[ret]   Returns EXIT_SHELL when the shell should stop running, 1 otherwise
*/
uint8 shellProcessByte(uint8 byte) {
	static uint8 bufferCount = 0;
    uint8 exitBool = 1;
    if(byte) {
        if((char)byte == '\r' || bufferCount == SHELL_BUFFER_SIZE - 1) {
            buffer[bufferCount] = '\0';
            exitBool = runCommand();
            buffer[0] = '\0';
            bufferCount = 0;
            usbSendString("\r<--> ");
        } else if (byte == 0x8) { // User pressed the backspace key
            buffer[--bufferCount] = '\0';
            usbSendString("\r<--> ");
            usbSendString(buffer);
        } else {
            buffer[bufferCount++] = byte;
            usbSendByte(byte);
        }
    }
    return exitBool;
}


//–––––– Command Enum ––––––//
enum commands {
    HELP = 1,
    HELLO,
    EXIT,
    SEND,
};


/*
[desc]  This function looks at the global shell buffer, and executes a command
		based on its contents. This function should be modified to serve whatever
		purpose the user needs. Create new commands in the enum above, and add their
		syntax to the determineCommand() function.
    
[ret]   Returns EXIT_SHELL when the shell should stop running, 1 otherwise
*/
uint8 runCommand(void) {
    char command[COMMAND_LENGTH] = {};
    char argument[ARGUMENT_LENGTH] = {};
    arrStruct response = {};
    uint8 ret = 1;
    
    if (sscanf(buffer, "%s", command)) {
        switch (determineCommand(command)) {
                
            case HELP:
                usbSendString("\r  Valid commands:");
                usbSendString("\r    send [command to send over I2C]");
                usbSendString("\r    help");
                break;
                
            case HELLO:
                usbSendString("\r  Hey there :)");
                break;
                
            case EXIT:
                usbSendString("\r  Exiting setup");
                ret = EXIT_SHELL;
                break;
                
            case SEND:
                sscanf(buffer, "%*s%s", argument);
                usbSendString("\r  Sent Command: ");
                usbSendString(argument);
                
                i2cSendString(EC_SENSOR_ADDRESS, argument);
                CyDelay(1200); //EZO requires one second for a reading
                response = i2cReadString(EC_SENSOR_ADDRESS);
                
                usbSendString("\r  received: ");
                usbSendString(response.d);
                break;
                
            default:
                usbSendString("   { Invalid Command }");
                break;
        } 
    }
    return ret;
}


/*
[desc]  Returns a value from the command enum based on the string passed in. 
		This function can be modified to support an arbitrary number of commands.

[command] A string to analyze for equivalence to a known command.
    
[ret]   Returns a value from the command enum if the input string matches a specified
		syntax, 0 otherwise.
*/
uint8 determineCommand(char command[]) {
    if(!strcmp(command, "hello")) {
        return HELLO;    
    } else if(!strcmp(command, "help")) {
        return HELP;
    } else if(!strcmp(command, "exit")) {
        return EXIT;
    } else if(!strcmp(command, "send")) {
        return SEND;
    } else {
        return 0;
    }
}


//EOF