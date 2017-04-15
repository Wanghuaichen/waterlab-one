
#include "project.h"

#include "waterlabSetupShell.h"
#include "usbProtocol.h"
#include "ezoProtocol.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    usbStart();
    LCD_Start();
    ezoStart();
    
    for (;;) {
        Test_Pin_Write(1);    
    }

    usbSendString("\r-- Welcome to the Waterlab One setup script --\r");
    usbSendString("Type 'help' to begin.");

    shellRun();
    
    for(;;);
}

/* [] END OF FILE */
