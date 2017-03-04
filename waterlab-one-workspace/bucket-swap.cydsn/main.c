/* 
    Carl Lindquist
    Feb 20, 2017
    
    I want to write a small state-machine which takes the input of two float switches,
    and uses their inputs to drive two relays.
    
*/
#include "project.h"
#include <stdlib.h>
#include <stdio.h>

#include "floatSwitch.h"

#define TRUE 1
#define FALSE 0


enum MACHINE_STATES {
    STATE_RUN_P1,
    STATE_RUN_P2,
} machineState;


uint8 negate(uint8 bool);

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    LCD_PrintString("Hello World");
    
    floatSwitchInit();
    
    machineState = STATE_RUN_P1;
    
    while(TRUE) {
        
        switch(machineState) {
            
            case STATE_RUN_P1:
                Pump1_Out_Pin_Write(TRUE);
                Pump2_Out_Pin_Write(FALSE);
            
                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 1");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(fswitchEvents);

                
                LED4_Pin_Write(TRUE);
                LED3_Pin_Write(FALSE);
                
                if (fsEventOccured(FSWITCH_EVENT_1_ON) || fsEventOccured(FSWITCH_EVENT_2_ON)) {
                    
                    machineState = STATE_RUN_P2;
                    fswitchEvents = FSWITCH_EVENT_NONE;
                }
                break;

            case STATE_RUN_P2:
                Pump1_Out_Pin_Write(FALSE);
                Pump2_Out_Pin_Write(TRUE);
                    
                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 2");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(fswitchEvents);

                
                LED4_Pin_Write(FALSE);
                LED3_Pin_Write(TRUE);
                
                if (fsEventOccured(FSWITCH_EVENT_3_ON) || fsEventOccured(FSWITCH_EVENT_0_ON)) {
                    
                    machineState = STATE_RUN_P1;
                    fswitchEvents = FSWITCH_EVENT_NONE;
                }
                break;
        }
        CyDelay(100);
    }
    
    
    
//    while(TRUE) {
//        
//        if (!Switch1_In_Pin_Read()) {
//            LED4_Pin_Write(TRUE);
//            Pump1_Out_Pin_Write(TRUE);
//        } else {
//            LED4_Pin_Write(TRUE);
//            Pump1_Out_Pin_Write(TRUE);
//        }
//        
//        if (!Switch2_In_Pin_Read()) {
//            LED3_Pin_Write(TRUE);
//            Pump2_Out_Pin_Write(TRUE);
//        } else {
//            LED3_Pin_Write(FALSE);
//            Pump2_Out_Pin_Write(FALSE);
//        }
//        CyDelay(10);
//    }
    
    for(;;); /* Just in case */
}





uint8 negate(uint8 bool) {
    if (bool == FALSE) {
        return TRUE;
    } else {
        return FALSE;
    }   
}


/* EOF */
