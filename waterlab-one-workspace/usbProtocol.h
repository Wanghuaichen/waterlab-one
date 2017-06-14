/* 
    Carl Lindquist
    Nov 15, 2016
    
    Interface to communicate over USBUART on the PSOC5 LP
*/

#ifndef USB_PROTOCOL_H
#define USB_PROTOCOL_H
    
#include <project.h>
    

/*
[desc]  Initializes USB, blocks until USB is ready.
		COM port must be connected for this function to finish
*/
void usbStart(void);
    

/*
[desc]  Returns the byte sent from the host PC over USBUART.

[ret]	Returns one byte of data if available, '\0' otherwise.
*/
uint8 usbGetByte(void);


/*
[desc]  Sends a byte to the host PC over USBUART.

[byte] Byte to be sent
*/
void usbSendByte(uint8 byte);


/*
[desc]  Sends a string to the host PC over USBUART. String not sent if
		length is greater than 63.

[string] String to be sent. Length should be less than 64.
*/
void usbSendString(char string[]);

/*
[desc]  Sends a string to the host PC over USBUART with some preformatting. 
        String not sent if length is greater than 63. Will send the string with
        the log level in square brackets before the string.
            Ex: '[loglevel] This is my string.'

[string] String to be sent. Length should be less than 64.
[logLevel] What sort of message this should be.
*/
void usbLog(char logLevel[], char string[]);

/*
[desc]  Sends an array of data to the host PC over USBUART. String not sent if
		length is greater than 63.

[string] Array of data to be sent.
[length] Length of the array.
*/
void usbSendData(uint8 data[], uint16 length);
    
#endif //USB_PROTOCOL_H