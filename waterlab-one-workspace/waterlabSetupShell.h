/* 
    Carl Lindquist
    Mar 17, 2017
    
    A shell for communication between the PSOC 5LP
    via USBUART for Waterlab Setup protocols.
    
    Pin Setup:
        USBUART Dm = P15[7]
        USBUART Dp = P15[6]
*/
    
#ifndef SHELL_H
#define SHELL_H

#include "project.h"
    
float highVoltThreshold;
float midVoltThreshold;
float currentThreshold;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Continuously runs the shell until the exit command is given. Constantly polls
        the USBUART for new bytes to process or store. The stored commands are processed
        when the user sends a '\r' from the USB host.
            Valid commands:

    	    	help
    	    	send [EZO I2C Command]
    	    	hello
                exit
*/
void shellRun(void);


/*
[desc]  Toggles the state of recirculation regarding the UV device. This is defined here
        so that the setupShell can use this function if requested by the user.
*/
void toggleRecirculation(void);
    

#endif //SHELL_H