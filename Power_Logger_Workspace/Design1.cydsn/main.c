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

#define ADC_RANGE 6.144
#define ADC_RESOLUTION 4095

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
        adcVal = ADC_DelSig_Read16();
        if (adcVal > 0x0FFF) { //Cap ADC
            adcVal = 0x0FFF;
        }
        
        volts = ((float)adcVal / ADC_RESOLUTION) * ADC_RANGE * 2;
        sprintf(outstring, "Volts: %.1f", volts);
        LCD_ClearDisplay();
        LCD_PrintString(outstring);

        if (minTimerFlag) {
            //Store result in SD Card
            minTimerFlag = 0;
        }
        CyDelay(50);
    }
}

CY_ISR(Minute_Timer_ISR) {
    Minute_Timer_STATUS; //Clears Isr
    minTimerFlag = 1;
}

/* EOF */
