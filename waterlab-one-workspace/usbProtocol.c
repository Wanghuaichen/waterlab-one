/*
    Carl Lindquist
    Nov 15, 2016
    
    Control code to communicate over USBUART on the PSOC5 LP
*/


#include <project.h>
#include <string.h>
#include <stdio.h>
#include "usbProtocol.h"

#define MAX_USB_STRING_LENGTH 63

uint8 initialized;

//––––––  Private Declarations  ––––––//

void usbStart(void);
uint8 usbGetByte(void);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void usbStart(void) {
    CyGlobalIntEnable; //Enable PSOC 5LP interrupts
    USBUART_Start(0u, USBUART_5V_OPERATION);
    while (!USBUART_GetConfiguration()){};
    USBUART_CDC_Init();
    initialized = 1;
}

uint8 usbGetByte(void) {
    uint8 byte = '\0';
    if (USBUART_DataIsReady()) {
        USBUART_GetAll(&byte);
    }
    return byte;
}

void usbSendByte(uint8 byte) {
    while(!USBUART_CDCIsReady());
    USBUART_PutChar(byte);
}

void usbSendString(char string[]) {
    if (initialized) {
        uint8 length = strlen(string);
        if(length > 0 && length <= MAX_USB_STRING_LENGTH) {
            while(!USBUART_CDCIsReady());
            USBUART_PutData((uint8*)string, length);
        }
    }
}

void usbSendData(uint8 data[], uint16 length) {
    if (initialized) {
        if(length > 0 && length <= MAX_USB_STRING_LENGTH) {
            while(!USBUART_CDCIsReady());
            USBUART_PutData(data, length);
        }     
    }
}

void usbLog(char logLevel[], char string[]) {
    if (initialized) {
        char outString[MAX_USB_STRING_LENGTH] = {};
        sprintf(outString, "\r[%s]-%s", logLevel, string);
        usbSendString(outString);
    }
}


//EOF