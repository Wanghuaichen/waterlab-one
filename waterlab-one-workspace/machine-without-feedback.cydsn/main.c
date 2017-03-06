/* 
    Carl Lindquist
    Feb 20, 2017
    
    First go at a state-machine without feedback.
    
*/
#include "project.h"
#include <stdlib.h>
#include <stdio.h>

#include "tank.h"

#define TRUE 1
#define FALSE 0


enum MACHINE_STATES {
    STATE_RUN_P0,
    STATE_RUN_P1,
    STATE_IDLE,
} machineState;


int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    LCD_PrintString("Hello World");
    
    tankInit();
    machineState = STATE_IDLE; //IDLE is used as the 'next state' picker
    
    tankStruct tankStates = tankGetStates();
    char outString[30] = {};
    while(TRUE) {
        
        switch(machineState) {
            
            case STATE_IDLE:
                Pump0_Out_Pin_Write(FALSE);
                Pump1_Out_Pin_Write(FALSE);
            
                LCD_ClearDisplay();
                LCD_PrintString("IDLE");
                sprintf(outString, "0:%d 1:%d 2:%d 3:%d", tankStates.tank[0], tankStates.tank[1] , tankStates.tank[2], tankStates.tank[3]);
                LCD_Position(1, 0);
                LCD_PrintString(outString);

                LED4_Pin_Write(TRUE);
                LED3_Pin_Write(TRUE);
                
                tankStates = tankGetStates();
                if ( (tankStates.tank[0] == TANK_STATE_MID || tankStates.tank[0] == TANK_STATE_FULL) 
                            && tankStates.tank[1] != TANK_STATE_FULL) {
                    machineState = STATE_RUN_P0;
                } else if ( (tankStates.tank[1] == TANK_STATE_MID || tankStates.tank[1] == TANK_STATE_FULL)
                            && tankStates.tank[0] != TANK_STATE_FULL) {
                    machineState = STATE_RUN_P1;
                }
                break;
            
            case STATE_RUN_P0:
                Pump0_Out_Pin_Write(TRUE);
                Pump1_Out_Pin_Write(FALSE);
            
                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 0");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(tankEvents);
                
                if (tankEventOccured(TANK_EVENT_0_EMPTY) || tankEventOccured(TANK_EVENT_1_FULL)) {
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;

            case STATE_RUN_P1:
                Pump0_Out_Pin_Write(FALSE);
                Pump1_Out_Pin_Write(TRUE);
                    
                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 1");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(tankEvents);
                
                if (tankEventOccured(TANK_EVENT_1_EMPTY) || tankEventOccured(TANK_EVENT_0_FULL)) {
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;
        }
        CyDelay(100); //For LCD printing purposes
    }
    
    
    for(;;); /* Just in case */
}

/* EOF */
