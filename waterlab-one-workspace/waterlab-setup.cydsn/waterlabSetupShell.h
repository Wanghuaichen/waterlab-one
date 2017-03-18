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

#include <project.h>
#include "usbProtocol.h"
    
#define EXIT_SHELL 0


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Continuously runs the shell until the exit command is given. Constantly polls
        the USBUART for new bytes to process or store. The stored commands are processed
        when the user types a '\r' on the USB host.
            Valid commands:

        	    	help
        	    	hello
                    exit
*/
void shellRun(void);
    

#endif //SHELL_H