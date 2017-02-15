/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

/* (Range - bypass buffer - inputDrain) * +/- range */
#define ADC_RANGE ((6.144 - 0.190) * 2) 
#define ADC_RESOLUTION 262144 // 2^n

CY_ISR_PROTO(Minute_Timer_ISR);



uint32 adcVal;
uint8 minTimerFlag;


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    LCD_Start();
    ADC_DelSig_Start();
    Minute_Timer_Start();
    Minute_Timer_Interrupt_StartEx(Minute_Timer_ISR);
    
    adcVal = 0;
    double volts = 0;
    minTimerFlag = 0;
    
    char outstring[16] = {};
    while(1) {
        
        adcVal = ADC_DelSig_Read32();
        volts = ((float)adcVal / ADC_RESOLUTION) * ADC_RANGE;
        
        sprintf(outstring, "Volts: %.3f", volts);
        LCD_ClearDisplay();
        LCD_PrintString(outstring);
        LCD_Position(1, 0);
        LCD_PrintString("Acc: +/-1[mV]");

        if (minTimerFlag) {
            //Store result in SD Card
            minTimerFlag = 0;
        }
        CyDelay(100);
    }
}

CY_ISR(Minute_Timer_ISR) {
    Minute_Timer_STATUS; //Clears Isr
    minTimerFlag = 1;
}

/* EOF */
