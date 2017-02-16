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
    
[fileName] The name of your file. Will be appended with -iteration. Ex: filename-22.txt
[header] A string which will be the first line of the file. 
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdStart(char fileName[], char header[]);

/*
[desc]	Appends a formatted data string to the data file.

[data] A double to append on a new line.
[precision] Decimal point precision for data to be written.
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdWriteData(double data, uint8 precision);

/*
[desc]	Appends a string to the data file.

[string] A string to append on a new line.
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdWriteDataString(char string[]);

    
#endif //SD_CARD_H
