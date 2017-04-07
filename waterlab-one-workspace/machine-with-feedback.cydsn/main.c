/* 
    Carl Lindquist
    Apr 6, 2017
    
    First go at a state-machine with feedback.
    
*/
#include "project.h"
#include <stdlib.h>
#include <stdio.h>

#include "tank.h"
#include "ezoProtocol.h"

#define TRUE 1
#define FALSE 0

#define EC_THRESHOLD 150.0
#define DO_THRESHOLD 150.0


enum MACHINE_STATES {
    STATE_IDLE,
    STATE_RUN_P0,
    STATE_RUN_P1,
    STATE_RUN_P2,
    STATE_RECIRCULATE_UV,
} machineState;

uint8 ecThresholdFlag;


int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    LCD_PrintString("Hello World");
    
    tankInit();
    ezoStart();
    machineState = STATE_IDLE; //IDLE is used as the 'next state' picker
    ecThresholdFlag = FALSE;
    
    tankStruct tankStates = tankGetStates();
    char outString[30] = {};
    while(TRUE) {
        
        switch(machineState) {
            
            case STATE_IDLE:

            
                LCD_ClearDisplay();
                LCD_PrintString("IDLE");
                sprintf(outString, "0:%d 1:%d 2:%d 3:%d", tankStates.tank[0], tankStates.tank[1] , tankStates.tank[2], tankStates.tank[3]);
                LCD_Position(1, 0);
                LCD_PrintString(outString);
                
                if ( (tankStates.tank[0] == TANK_STATE_MID || tankStates.tank[0] == TANK_STATE_FULL) 
                            && tankStates.tank[1] != TANK_STATE_FULL) {
                    Pump0_Out_Pin_Write(TRUE);
                    machineState = STATE_RUN_P0;
                } else if ( (tankStates.tank[1] == TANK_STATE_MID || tankStates.tank[1] == TANK_STATE_FULL)
                            && tankStates.tank[2] != TANK_STATE_FULL) {
                    Pump1_Out_Pin_Write(TRUE);
                    machineState = STATE_RUN_P1;
                } else if ( (tankStates.tank[2] == TANK_STATE_MID || tankStates.tank[2] == TANK_STATE_FULL)
                            && tankStates.tank[3] != TANK_STATE_FULL) {
                    Pump2_Out_Pin_Write(TRUE);
                    machineState = STATE_RUN_P2;
                } else if (tankStates.tank[0] == TANK_STATE_FULL && tankStates.tank[1] == TANK_STATE_FULL
                            && tankStates.tank[2] == TANK_STATE_FULL && tankStates.tank[3] == TANK_STATE_FULL) {
                    //Open
                    machineState = STATE_RECIRCULATE_UV;    
                }
                break;
            
            case STATE_RUN_P0:

                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 0");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(tankEvents);
                
                if (tankEventOccured(TANK_EVENT_0_EMPTY) || tankEventOccured(TANK_EVENT_1_FULL)) {
                    Pump0_Out_Pin_Write(FALSE);
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;

            case STATE_RUN_P1:

                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 1");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(tankEvents);
                
                if (tankEventOccured(TANK_EVENT_1_EMPTY) || tankEventOccured(TANK_EVENT_0_FULL)) {
                    Pump1_Out_Pin_Write(FALSE);
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;
                
            case STATE_RUN_P2:
    
                LCD_ClearDisplay();
                LCD_PrintString("RUN PUMP 2");
                LCD_Position(1, 0);
                LCD_PrintHexUint16(tankEvents);
                
                if (tankEventOccured(TANK_EVENT_2_EMPTY) || tankEventOccured(TANK_EVENT_3_FULL)) {
                    Pump2_Out_Pin_Write(FALSE);
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;
                
            case STATE_RECIRCULATE_UV:
                LCD_ClearDisplay();
                LCD_PrintString("RECIRCULATE UV");
                if ( (tankEventOccured(TANK_EVENT_1_EMPTY) || tankEventOccured(TANK_EVENT_2_EMPTY) || tankEventOccured(TANK_EVENT_3_EMPTY))
                        && ecThresholdFlag == FALSE) {
                    Pump3_Out_Pin_Write(FALSE);
                    machineState = STATE_IDLE;
                }
                break;
                
                
        }
        tankStates = tankGetStates();
        if (ezoGetData(EC_SENSOR_ADDRESS) > EC_THRESHOLD) {
            ecThresholdFlag = TRUE;
        } else {
             ecThresholdFlag = FALSE;
        }
        
        if (ecThresholdFlag && tankStates.tank[3] != TANK_STATE_EMPTY && tankStates.tank[3] != TANK_STATE_UNDEF) {
            /* SWITCH SOLENOID WYE STATE HERE */
            
            Pump0_Out_Pin_Write(FALSE);
            Pump1_Out_Pin_Write(FALSE);
            Pump2_Out_Pin_Write(FALSE);
            Pump3_Out_Pin_Write(TRUE);
            machineState = STATE_RECIRCULATE_UV;    
        }
        
        if (ezoGetData(DO_SENSOR_ADDRESS) > DO_THRESHOLD) {
            /* TURN ON BUBBLER HERE */
        }
        CyDelay(25); //For LCD printing purposes
    }
    
    
    for(;;); /* Just in case */
}

/* EOF */
