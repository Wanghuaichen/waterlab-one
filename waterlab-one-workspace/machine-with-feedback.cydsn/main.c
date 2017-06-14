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
#include "usbProtocol.h"
#include "waterlabSetupShell.h"

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
uint8 filterStageActive, roStageActive, uvStageActive;
int32 dutyCycle;

int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    LCD_Start();
    ADC_Start();
    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    
    tankInit();
    ezoStart();
    
    
    
    /* Enter and run the Waterlab Setup Shell */
    if (!SW3_Pin_Read()) {
        usbStart(); /* Will not return until COMM port is connected */
        usbSendString("\r-- Welcome to the Waterlab One setup script --\r");
        usbSendString("Type 'help' to begin.");
        shellRun(); /* Will not return until 'exit\r' is received over USBUART */
    }
    
    Pump0_En_Write(1);
    for(;;);
    
    filterStageActive = FALSE;
    roStageActive = FALSE;
    uvStageActive = FALSE;
    
    tankStruct tankStates;
    char outString[30] = {};
    while(TRUE) {
        
        
        
        tankStates = tankGetStates();
        /* Activate pumps according to Active Devices */
        Pump0_En_Write(filterStageActive);
        CyDelayUs(1000);
        Pump1_En_Write(roStageActive);
        CyDelayUs(1000);
        Pump2_En_Write(uvStageActive);
        CyDelayUs(1000);
        
        
        /* Turn devices off if appropriate */
        if (tankClearEvent(TANK_EVENT_0_EMPTY) || tankClearEvent(TANK_EVENT_1_FULL)) {
            filterStageActive = FALSE;
            
        }
        if (tankClearEvent(TANK_EVENT_1_EMPTY) || tankClearEvent(TANK_EVENT_2_FULL)) {
            roStageActive = FALSE;    
        }
        if (tankClearEvent(TANK_EVENT_2_EMPTY) || tankClearEvent(TANK_EVENT_3_FULL)) {
            uvStageActive = FALSE;    
        }
        
        /* Turn devices on if appropriate */
        if ( (tankStates.tank[0] == TANK_STATE_MID || tankStates.tank[0] == TANK_STATE_FULL) && tankStates.tank[1] != TANK_STATE_FULL) {
            filterStageActive = TRUE;
        }
        if ( (tankStates.tank[1] == TANK_STATE_MID || tankStates.tank[1] == TANK_STATE_FULL) && tankStates.tank[2] != TANK_STATE_FULL) {
            roStageActive = TRUE;
        }
        if ( (tankStates.tank[2] == TANK_STATE_MID || tankStates.tank[2] == TANK_STATE_FULL) && tankStates.tank[3] != TANK_STATE_FULL) {
            uvStageActive = TRUE;
        }
        
        LCD_ClearDisplay();
        sprintf(outString, "ACTIVE: %d%d%d", filterStageActive, roStageActive, uvStageActive);
        LCD_PrintString(outString);
        
        
        dutyCycle = ADC_Read32() * 100 / 0xffff;
        if (dutyCycle > 100) {
            dutyCycle = 100;
        }
        LCD_Position(1, 0);
        sprintf(outString, "Duty: %ld", dutyCycle);
        LCD_PrintString(outString);
        PWM_0_WriteCompare(dutyCycle);
        PWM_1_WriteCompare(dutyCycle);
        PWM_2_WriteCompare(dutyCycle);
        
        
        CyDelay(50);
        
        while(tankStates.tank[0] == TANK_STATE_UNDEF || tankStates.tank[1] == TANK_STATE_UNDEF
              || tankStates.tank[2] == TANK_STATE_UNDEF || tankStates.tank[3] == TANK_STATE_UNDEF) {
                tankStates = tankGetStates();
            filterStageActive = FALSE;
            roStageActive = FALSE;
            uvStageActive = FALSE;
            Pump0_En_Write(filterStageActive);
            Pump1_En_Write(roStageActive);
            Pump2_En_Write(uvStageActive);
                
            LED3_Pin_Write(TRUE);
            LED4_Pin_Write(TRUE);
            LCD_ClearDisplay();
            LED3_Pin_Write(FALSE);
            LED4_Pin_Write(FALSE);
        }
    }
}



#ifdef OUTDATED

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
                
                if (tankEventoccurred(TANK_EVENT_0_EMPTY) || tankEventoccurred(TANK_EVENT_1_FULL)) {
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
                
                if (tankEventoccurred(TANK_EVENT_1_EMPTY) || tankEventoccurred(TANK_EVENT_0_FULL)) {
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
                
                if (tankEventoccurred(TANK_EVENT_2_EMPTY) || tankEventoccurred(TANK_EVENT_3_FULL)) {
                    Pump2_Out_Pin_Write(FALSE);
                    machineState = STATE_IDLE;
                    tankEvents = TANK_EVENT_NONE;
                }
                break;
                
            case STATE_RECIRCULATE_UV:
                LCD_ClearDisplay();
                LCD_PrintString("RECIRCULATE UV");
                if ( (tankEventoccurred(TANK_EVENT_1_EMPTY) || tankEventoccurred(TANK_EVENT_2_EMPTY) || tankEventoccurred(TANK_EVENT_3_EMPTY))
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

#endif //Outdated

/* EOF */
