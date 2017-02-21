/* 
    Carl Lindquist
    Feb 20, 2017
    
    I want to write a small state-machine which takes the input of two float switches,
    and uses their inputs to drive two relays.
    
*/
#include "project.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    LCD_PrintString("Hello World");
    
    while(1) {
        
        if (Switch1_In_Pin_Read()) {
            LED3_Pin_Write(1);
        } else {
            LED3_Pin_Write(0);
        }
        CyDelay(10);
    }
    
    for(;;); /* Just in case */
}

/* EOF */
