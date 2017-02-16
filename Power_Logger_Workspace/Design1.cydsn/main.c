/*
    Carl Lindquist
    February 12, 2017
    
    Data logger for the PSoC 5LP. Writes values to an SD card
    every timerInterrupt. Waits for the user to insert an SD on 
    startup.
*/
#include "project.h"
#include <stdio.h>

#include "sdCard.h"

/* (Range - bypass buffer - inputDrain) * +/- range */
#define ADC_RANGE ((6.144 - 0.190) * 2) 
#define ADC_RESOLUTION 262144 // 2^n
#define DATA_PRECISION 3
#define MAX_VOLTAGE 6.0

CY_ISR_PROTO(Logger_Timer_ISR);

uint32 adcVal;
uint8 loggerTimerFlag;


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    LCD_Start();
    while (!sdStart("data", "1 min timestep")) {
        LCD_ClearDisplay();
        LCD_PrintString("SD ERROR:");
        LCD_Position(1, 0);
        LCD_PrintString("Card plugged in?");
        CyDelay(500);
    }
    ADC_DelSig_Start();
    Logger_Timer_Start();
    Logger_Timer_Interrupt_StartEx(Logger_Timer_ISR);
    
    adcVal = 0;
    loggerTimerFlag = 0;
    double volts = 0;
    uint32 dataPoints = 0;
    uint32 errorPoints = 0;
    
    char outstring[16] = {};
    while(1) {
        
        adcVal = ADC_DelSig_Read32();
        volts = ((float)adcVal / ADC_RESOLUTION) * ADC_RANGE;
        
        sprintf(outstring, "Volts: %.*f", DATA_PRECISION, volts);
        LCD_ClearDisplay();
        LCD_PrintString(outstring);
        LCD_Position(1, 0);
        sprintf(outstring, "D: %lu E: %lu", dataPoints, errorPoints);
        LCD_PrintString(outstring);
        
        /* Log voltage if min */
        if (loggerTimerFlag && volts < MAX_VOLTAGE) {
            if (!sdWriteData(volts, DATA_PRECISION)) {
                errorPoints++;
                LCD_ClearDisplay();
                LCD_PrintString("SD ERROR:");
                LCD_Position(1, 0);
                LCD_PrintString("Could not write data.");
            } else {
                dataPoints++;
            }
            loggerTimerFlag = 0;
        }
        CyDelay(100);
    }
    
    for(;;); //In case we break
}

CY_ISR(Logger_Timer_ISR) {
    Logger_Timer_STATUS; //Clears Isr
    loggerTimerFlag = 1;
}

/* EOF */
