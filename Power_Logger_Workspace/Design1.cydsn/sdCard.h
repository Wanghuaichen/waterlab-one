/* 
    Carl Lindquist
    Nov 29, 2016
    
    Interface for logging data to an SD card.
*/
    
#ifndef SD_CARD_H
#define SD_CARD_H

#include <project.h>
    
    
    
/*
[desc]	Creates a new .txt file and initializes it with startString.
        Automatically appends a newline if startString has length > 0.
    
[filePrefix] Filename; will be appended with -iteration. Ex: filename-22.txt
[startString] A string which will be the first line of the file. 
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdStart(char filePrefix[], char startString[]);


/*
[desc]	Appends a formatted data string to the data file.

[data] A double to append on a new line.
[precision] Decimal point precision for data to be written.
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdWriteData(double data, uint8 precision);

    
#endif //SD_CARD_H
