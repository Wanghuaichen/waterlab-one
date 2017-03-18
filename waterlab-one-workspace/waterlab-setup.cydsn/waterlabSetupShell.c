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
#define COMMAND_LENGTH 16
#define ARGUMENT_LENGTH 16

uint8 bufferCount = 0;
char buffer[SHELL_BUFFER_SIZE];

//–––––– Private Declarations ––––––//

uint8 shellProcessByte(uint8 byte);
uint8 runCommand(void);
uint8 determineCommand(char command[]);

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//


void shellRun(void) {
    usbSendString("<--> ");
    while (shellProcessByte(usbGetByte()));    
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Input bytes are buffered until '\r' received or buffer is full. Upon execute,
		shell checks buffer for a valid command.
        
[byte]	Bytes to be buffered, '\r' executes buffer.
    
[ret]   Returns EXIT_SHELL when the shell should stop running, 1 otherwise
*/
uint8 shellProcessByte(uint8 byte) {
    uint8 ret = 1;
    if(byte) {
        if((char)byte == '\r' || bufferCount == SHELL_BUFFER_SIZE) {
            buffer[bufferCount] = '\0';
            ret = runCommand();
            usbSendString("\r<--> ");
            buffer[0] = '\0';
            bufferCount = 0;
        } else if (byte == 0x8) { // User pressed the backspace key
            buffer[--bufferCount] = '\0';
            usbSendString("\r<--> ");
            usbSendString(buffer);
        } else {
            buffer[bufferCount++] = byte;
            usbSendByte(byte);
        }
    }
    return ret;
}


enum commands {
    HELP = 1,
    HELLO,
    EXIT,
    SEND,
};


//executes a command based on the return of determineCommand()
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
                
                i2cSendString(100, argument);
                CyDelay(1200);
                response = i2cReadString(100);
                
                usbSendString("\r  received: ");
                usbSendString(response.r);
                break;
                
            default:
                usbSendString("   { Invalid Command }");
                break;
        } 
    }
    return ret;
}


//Returns which command should be executed based on the string input
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