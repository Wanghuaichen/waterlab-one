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
#define SHELL_PROMPT_STRING ("\r<--> ")
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
    usbSendString(SHELL_PROMPT_STRING);
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
            usbSendString(SHELL_PROMPT_STRING);
        } else if (byte == 0x8) { // User pressed the backspace key
            buffer[--bufferCount] = '\0';
            usbSendString(SHELL_PROMPT_STRING);
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
    EXIT,
    SEND,
    SET_AUTO_POLL,
    CHANGE_ACTIVE_DEVICE,
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
    static uint8 activeDevice = EC_SENSOR_ADDRESS;
    uint8 ret = 1;
    
    if (sscanf(buffer, "%s", command)) {
        switch (determineCommand(command)) {
                
            case HELP:
                usbSendString("\r  Active Device: ");
                if (activeDevice == EC_SENSOR_ADDRESS) {
                    usbSendString("Electrical Conductivity Sensor");
                } else {
                    usbSendString("Dissolved Oxygen Sensor");
                }
                
                usbSendString("\r  Valid commands:");
                usbSendString("\r    send [command]");
                usbSendString("\r      Sends 'command' over I2C to the 'Active Device'. The sensor");
                usbSendString("\r      will respond after a 1 second delay. The first character received");
                usbSendString("\r      is a response code:");
                usbSendString("\r        S - Success");
                usbSendString("\r        E - Error, invalid EZO command");
                usbSendString("\r        N - No data requested");
                usbSendString("\r        P - Pending data, request delay not long enough");
                
                usbSendString("\r    set_auto_poll ['on' or 'off'] ");
                usbSendString("\r      Enables/disables autopolling for data.");
                
                usbSendString("\r    change_active_device");
                usbSendString("\r      Toggles the 'Active Device'.");
                
                usbSendString("\r    exit");
                usbSendString("\r      Exit this shell.");
                
                usbSendString("\r    help");
                break;
                
            case EXIT:
                usbSendString("\r  Exiting setup");
                ret = EXIT_SHELL;
                break;
                
            case SEND:
                sscanf(buffer, "%*s%s", argument);
                if (activeDevice == EC_SENSOR_ADDRESS) {
                    usbSendString("\r  Sent Command to EC Sensor: ");
                } else if (activeDevice == DO_SENSOR_ADDRESS) {
                    usbSendString("\r  Sent Command to DO Sensor: ");    
                }
                usbSendString(argument);
                
                response = ezoSendAndPoll(activeDevice, argument, 1200);
                
                usbSendString("\r  Received: ");
                usbSendString(response.d);
                break;
                
            case SET_AUTO_POLL:
                sscanf(buffer, "%*s%s", argument);
                if (!strcmp(argument, "off")) {
                    ezoSetAutoPoll(0);
                    usbSendString("\r  Turned auto polling OFF");
                } else {
                    ezoSetAutoPoll(1);
                    usbSendString("\r  Turned auto polling ON");
                }
                break;
                
            case CHANGE_ACTIVE_DEVICE:
                if (activeDevice == EC_SENSOR_ADDRESS) {
                    activeDevice = DO_SENSOR_ADDRESS;
                    usbSendString("\r  Made waterlabDO the active device");
                } else {
                    activeDevice = EC_SENSOR_ADDRESS;
                    usbSendString("\r  Made waterlabEC the active device");
                }
                break;
                
            default:
                usbSendString("   { Invalid Shell Command }");
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
    if(!strcmp(command, "help")) {
        return HELP;
    } else if(!strcmp(command, "exit")) {
        return EXIT;
    } else if(!strcmp(command, "send")) {
        return SEND;
    } else if(!strcmp(command, "set_auto_poll")) {
        return SET_AUTO_POLL;
    } else if(!strcmp(command, "change_active_device")) {
        return CHANGE_ACTIVE_DEVICE;
    } else {
        return 0;
    }
}


//EOF