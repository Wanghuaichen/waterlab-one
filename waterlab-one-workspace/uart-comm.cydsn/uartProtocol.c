/*
   Carl Lindquist
   Mar 9, 2017
*/

#include "uartProtocol.h"
#include <stdio.h>



#define BUFFER_SIZE 128


uint8 rxBuffer[BUFFER_SIZE];



//–––––––––– Private Declarations ––––––––––//

void uartClearRxBuffer(void);
CY_ISR_PROTO(rxIsr);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void uartStart(void) {
    CyGlobalIntEnable 
    UART_Start();
    validPacketReceived = 0;
    UART_Rx_Isr_StartEx(rxIsr);
}

void uartSendString(uint8 string[]) {
    UART_PutString((char*)string);
}

responseStruct uartGetResponse(void) {
    if (validPacketReceived) {
        sprintf((char*)response.r, "%s", rxBuffer);
        uartClearRxBuffer();
    }
    return response;
}

double uartGetData(void) {
    if (!validPacketReceived) {
        return PACKET_NOT_READY;
    }
    double data;
    sscanf((char*)rxBuffer, "%lf", &data);
    return data;
}




//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

void uartClearRxBuffer(void) {
    uint8 i = 0;
    for(; i < BUFFER_SIZE; i++) {
        rxBuffer[i] = '\0';    
    }
}

CY_ISR(rxIsr) { //BYTE RECEIVED
    static uint8 rxCount = 0;
    uint8 byte = UART_GetChar();
    rxBuffer[rxCount++] = byte;
    if (byte == '\r') {
        rxBuffer[rxCount - 1] = '\0';
        sprintf((char*)response.r, "%s", rxBuffer);
        validPacketReceived = 1;
        uartClearRxBuffer();
        rxCount = 0;
    }
}
