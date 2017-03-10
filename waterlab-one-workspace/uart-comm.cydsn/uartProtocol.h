/* 
    Carl Lindquist
    Mar 9, 2017
*/
    
#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "project.h"
    
#define PACKET_NOT_READY -1.0
    
typedef struct responsStruct { uint8 r[128]; } responseStruct;

responseStruct response;
uint8 validPacketReceived;

responseStruct uartGetResponse(void);

void uartSendString(uint8 string[]);

void uartStart(void);

double uartGetData(void);


#endif /* UART_PROTOCOL_H */
