/*
    Carl Lindquist
    Mar 9, 2017
*/


#include "project.h"
#include "uartProtocol.h"

int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    LCD_PrintString("Some goodies");
    CyDelay(1000);
    
    uartStart();
    
    uartSendString((uint8*)"Status\r");
    for(;;) {
        if (validPacketReceived) {
            validPacketReceived = 0;
            LCD_ClearDisplay();
            LCD_PrintString((char*)response.r);
            CyDelay(50);
            uartSendString((uint8*)"Status\r");
        }
        
    }
}

/* [] END OF FILE */
