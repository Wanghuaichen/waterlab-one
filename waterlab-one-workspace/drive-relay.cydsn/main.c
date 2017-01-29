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
#include <project.h>
#define TRUE 1
#define FALSE 0

int main()
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    int adcVal = 0;
    ADC_Start();

    while(TRUE) {
        adcVal = ADC_Read16();
        if(adcVal < 1000) {
            Control_Reg_Write(TRUE);
        } else {
            Control_Reg_Write(FALSE);
        }
        CyDelay(1);
    }
}

/* [] END OF FILE */
