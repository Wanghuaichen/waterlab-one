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
#include "pressure.h"
#include "usbProtocol.h"
#include "waterlabSetupShell.h"
#include "tristarProtocol.h"

#define TRUE 1
#define FALSE 0

#define EC_THRESHOLD 150.0
#define DO_THRESHOLD 150.0

#define PSENSOR_ZERO_THRESHOLD 40.5
#define PSENSOR_ONE_THRESHOLD 90.0

#define HIGH_POWER_VOLT_THRESHOLD 23.6
#define MID_POWER_VOLT_THRESHOLD 23.4
#define HIGH_POWER_CURRENT_THRESHOLD 40.6

#define TIMER_RECIRCULATE_THREE_HOURS 1080000000
#define TIMER_RECIRCULATE_FIVE_HOURS 1800000000



enum MID_POWER_STATES {
    STATE_IDLE,
    STATE_RUN_P0,
    STATE_RUN_P1,
    STATE_RUN_P2,
    STATE_RECIRCULATE_UV,
}midPowerState;


enum POWER_MODES {
    HIGH_POWER_MODE,
    MID_POWER_MODE,
    LOW_POWER_MODE,
} powerMode;

uint8 recirculate;
int32 dutyCycle;

//––––––  Private Declarations  ––––––//
CY_ISR_PROTO(Watchdog_ISR);
CY_ISR_PROTO(Recirculate_Isr);

uint8 getDutyCycle(uint8 potIndex);
void runHighPower(void);
void runMidPower(void);
void midPowerInit(void);
void lowPowerInit(void);


int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    Timer_Recirculate_Start();
    Timer_Recirculate_Sleep();
    Recirculate_Interrupt_StartEx(Recirculate_Isr);
    Solenoid_Select_Write(0);
    Solenoid_Signal_Write(1);
    CyDelay(50); //Length of signal to toggle solenoid
    Solenoid_Signal_Write(0);
    
    Timer_Watchdog_Start();
    Watchdog_Interrupt_StartEx(Watchdog_ISR);
    
    ADC_Pot_Start();
    ADC_Pot_StartConvert();
    PWM_0_Start();
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    
    tankInit();
    ezoStart();
    pressureInit();
    tstarStart();
    
    usbStart();
    uint8 data[4] = {0x00, 0x00, 0x00, 0x02};
    while(1) {
        if (!SW1_Pin_Read()) {
            tstarSendData(READ_INPUT_REG, data, 4);
            CyDelay(1000);
        }
    }
    
    highVoltThreshold = HIGH_POWER_VOLT_THRESHOLD;
    midVoltThreshold = MID_POWER_VOLT_THRESHOLD;
    currentThreshold = HIGH_POWER_CURRENT_THRESHOLD;
    
    /* Enter and run the Waterlab Setup Shell */
    
    if (!SW1_Pin_Read()) {
        LED_Pin_Write(1);
        usbStart(); /* Will not return until COMM port is connected */
        usbSendString("\r-- Welcome to the Waterlab One setup script --\r");
        usbSendString("Type 'help' to begin.");
        
        shellRun(); /* Will not return until 'exit\r' is received over USBUART */
    }
    
    powerMode = MID_POWER_MODE;
    tankStruct tankStates;
    while(TRUE) {
        
        switch (powerMode) {
        
            case HIGH_POWER_MODE:
                runHighPower();
                if (FALSE /* panelCurrent < HIGH_POWER_CURRENT_THRESHOLD || battVoltage < HIGH_POWER_VOLT_THRESHOLD */) {
                    midPowerInit();
                    powerMode = MID_POWER_MODE;    
                }
                break;
                    
                    
            case MID_POWER_MODE:    
                runMidPower();
                if (FALSE /* panelCurrent > HIGH_POWER_CURRENT_THRESHOLD && battVoltage > HIGH_POWER_VOLT_THRESHOLD */) {
                    powerMode = HIGH_POWER_MODE;
                } else if (FALSE /* battVoltage < MID_POWER_VOLT_THRESHOLD */) {
                    lowPowerInit();
                    powerMode = LOW_POWER_MODE;
                }
                break;
                
            case LOW_POWER_MODE:
                if (FALSE /* battVoltage > MID_POWER_VOLT_THRESHOLD */) {
                    Timer_Recirculate_Sleep();
                    Pump2_En_Write(FALSE);
                    UV_En_Write(FALSE);
                    toggleRecirculation();
                    
                    midPowerInit();
                    powerMode = MID_POWER_MODE;
                }
                break;
        }

        
        /* In case you find an undefined state! */
        while(tankStates.tank[0] == TANK_STATE_UNDEF || tankStates.tank[1] == TANK_STATE_UNDEF
                || tankStates.tank[2] == TANK_STATE_UNDEF || tankStates.tank[3] == TANK_STATE_UNDEF) {
            usbLog("Warning", "Undefined float switch state");
            tankStates = tankGetStates();
            Pump0_En_Write(FALSE);
            Pump1_En_Write(FALSE);
            Pump2_En_Write(FALSE);
            UV_En_Write(FALSE);
            Bubbler_En_Write(FALSE);
            CyDelay(1000);
        }
                

        while(ezoGetData(EC_SENSOR_ADDRESS) > EC_THRESHOLD) {
            usbLog("Warning", "EC threshold exceeded");
            LED_Pin_Write(1);
            Pump0_En_Write(FALSE);
            Pump1_En_Write(FALSE);
            Pump2_En_Write(FALSE);
            UV_En_Write(FALSE);
            CyDelay(1000);
        }
        
        while(FALSE /*getPressure(PSENSOR_ZERO) > PSENSOR_ZERO_THRESHOLD */) {
            usbLog("Warning", "Microfilter pressure threshold exceeded");
            LED_Pin_Write(1);
            Pump0_En_Write(FALSE);
            Pump1_En_Write(FALSE);
            Pump2_En_Write(FALSE);
            UV_En_Write(FALSE);
            CyDelay(1000);
        }
        
        while(FALSE /*getPressure(PSENSOR_ONE) > PSENSOR_ONE_THRESHOLD */) {
            usbLog("Warning", "RO pressure threshold exceeded");
            LED_Pin_Write(1);
            Pump0_En_Write(FALSE);
            Pump1_En_Write(FALSE);
            Pump2_En_Write(FALSE);
            UV_En_Write(FALSE);
            CyDelay(1000);
        }
        LED_Pin_Write(0);
    }
    
   
}


void runHighPower(void) {
    tankStruct tankStates = tankGetStates();
    uint8 filterStageActive, roStageActive, uvStageActive;
    
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
    
    /* Activate pumps according to Active Devices */
    Pump0_En_Write(filterStageActive);
    CyDelayUs(1000);
    Pump1_En_Write(roStageActive);
    CyDelayUs(1000);
    Pump2_En_Write(uvStageActive);
    UV_En_Write(uvStageActive);
    CyDelayUs(1000);
    
    PWM_0_WriteCompare(getDutyCycle(0));
    PWM_1_WriteCompare(getDutyCycle(1));
    PWM_2_WriteCompare(getDutyCycle(2));
}

void midPowerInit(void) {
    PWM_0_WriteCompare(100);
    PWM_1_WriteCompare(100);
    PWM_2_WriteCompare(100);
    midPowerState = STATE_IDLE;//IDLE is used as the 'next state' picker
}

void runMidPower(void) {
    
    tankStruct tankStates = tankGetStates();
    switch(midPowerState) {
            
        case STATE_IDLE:

            if ( (tankStates.tank[0] == TANK_STATE_MID || tankStates.tank[0] == TANK_STATE_FULL) 
                        && tankStates.tank[1] != TANK_STATE_FULL) {
                midPowerState = STATE_RUN_P0;
            } else if ( (tankStates.tank[1] == TANK_STATE_MID || tankStates.tank[1] == TANK_STATE_FULL)
                        && tankStates.tank[2] != TANK_STATE_FULL) {
                midPowerState = STATE_RUN_P1;
            } else if ( (tankStates.tank[2] == TANK_STATE_MID || tankStates.tank[2] == TANK_STATE_FULL)
                        && tankStates.tank[3] != TANK_STATE_FULL) {
                midPowerState = STATE_RUN_P2;
            } else if ( tankStates.tank[0] == TANK_STATE_EMPTY && tankStates.tank[1] == TANK_STATE_FULL
                        && tankStates.tank[2] == TANK_STATE_FULL && tankStates.tank[3] != TANK_STATE_EMPTY) {
                midPowerState = STATE_RECIRCULATE_UV;
                toggleRecirculation();
            }
            break;
        
        case STATE_RUN_P0:
            Pump0_En_Write(TRUE);
            if (tankClearEvent(TANK_EVENT_0_EMPTY) || tankClearEvent(TANK_EVENT_1_FULL)) {
                Pump0_En_Write(FALSE);
                midPowerState = STATE_IDLE;
            }
            break;

        case STATE_RUN_P1:
            Pump1_En_Write(TRUE);
            if (tankClearEvent(TANK_EVENT_1_EMPTY) || tankClearEvent(TANK_EVENT_2_FULL)) {
                Pump1_En_Write(FALSE);
                midPowerState = STATE_IDLE;
            }
            break;
            
        case STATE_RUN_P2:

            Pump2_En_Write(TRUE);
            UV_En_Write(TRUE);
            if (tankClearEvent(TANK_EVENT_2_EMPTY) || tankClearEvent(TANK_EVENT_3_FULL)) {
                Pump2_En_Write(FALSE);
                UV_En_Write(FALSE);
                midPowerState = STATE_IDLE;
            }
            break;
            
        case STATE_RECIRCULATE_UV:
            Pump2_En_Write(TRUE);
            UV_En_Write(TRUE);
            if (tankStates.tank[3] != TANK_STATE_FULL && tankStates.tank[3] != TANK_STATE_UNDEF) {
                Pump2_En_Write(FALSE);
                UV_En_Write(FALSE);
                toggleRecirculation();
                midPowerState = STATE_IDLE;
            }
            break;
 
    }
}

void lowPowerInit(void) {
    PWM_0_WriteCompare(100);
    PWM_1_WriteCompare(100);
    PWM_2_WriteCompare(100);
    
    Timer_Recirculate_Wakeup();
    Timer_Recirculate_WriteCounter(0);
    Timer_Recirculate_WritePeriod(TIMER_RECIRCULATE_THREE_HOURS);
    toggleRecirculation();
    
    Pump2_En_Write(TRUE);
    UV_En_Write(TRUE);
}

CY_ISR(Watchdog_ISR) {
    /* Turn on bubbler if DO exceeds threshold */
    Bubbler_En_Write(ezoGetData(DO_SENSOR_ADDRESS) > DO_THRESHOLD);
    
    Timer_Watchdog_STATUS;
}

CY_ISR(Recirculate_Isr) {

    if (Pump2_En_Read()) {
        Pump2_En_Write(FALSE);
        UV_En_Write(FALSE);
        Timer_Recirculate_WriteCounter(0);
        Timer_Recirculate_WritePeriod(TIMER_RECIRCULATE_FIVE_HOURS);
    } else {
        Pump2_En_Write(TRUE);
        UV_En_Write(TRUE);
        Timer_Recirculate_WriteCounter(0);
        Timer_Recirculate_WritePeriod(TIMER_RECIRCULATE_THREE_HOURS);
    }

    Timer_Recirculate_STATUS;   
}


uint8 getDutyCycle(uint8 potIndex) {
    AMux_Pot_Select(potIndex);
    ADC_Pot_IsEndConversion(ADC_Pot_WAIT_FOR_RESULT);
    uint8 dutyCycle = ADC_Pot_GetResult8() * 100 / 0xff;
    if (dutyCycle > 100) {
        dutyCycle = 100;
    }
    return dutyCycle;
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
