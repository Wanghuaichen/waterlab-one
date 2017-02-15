/* 
    Carl Lindquist
    Feb 14, 2017 
    
    Control code for logging data to an SD card.
*/
    
#include "sdCard.h"
#include <FS.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_NAME_LENGTH 50

    
uint8 initSuccessful;

//FS_FILE* dataFile;
char dataFileName[MAX_FILE_NAME_LENGTH];
    

//––––––  Private Declarations  ––––––//
uint8 sdAppendString(char fileName[], char string[]);
uint8 sdClose(FS_FILE* file);
    

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

uint8 sdStart(char fileName[], char header[]) {
    FS_Init();
    FS_FAT_SupportLFN(); /* Support long file names */
    initSuccessful = 1;
    
    /* Figure out new file number, and record filename */
    uint16 fileNum = 0;
    do {
        sprintf(dataFileName, "%s-%d.txt", fileName, fileNum);
        fileNum++;
    } while(FS_FOpen(dataFileName, "r"));

    FS_FILE* dataFile = FS_FOpen(dataFileName, "w");
    if(!dataFile) {
        initSuccessful = 0;
        return 0;
    }

    int strLen = strlen(header);
    if (strLen) {
        FS_Write(dataFile, header, strLen);
        FS_Write(dataFile, "\r\n", 2);   
    }
    
    sdClose(dataFile);
    return 1;
}

uint8 sdWriteData(double data, uint8 precision) {
    char dataString[100] = {};
    sprintf(dataString, "%.*f\r\n", precision, data);
    if (sdAppendString(dataFileName, dataString)) {
        return 1;   
    } else {
        return 0;
    }
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]	Appends a string to a file. String must have length > 0.
    
[filename] File to be appended.
[string] A string to append.
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdAppendString(char fileName[], char string[]) {
    if(!initSuccessful) {
        return 0;    
    }
    FS_FILE* openFile = FS_FOpen(fileName, "a");

    uint16 length = 0;
    for(; string[length]; length++);
    if(length && FS_Write(openFile, string, length)) {
        sdClose(openFile);
        return 1;    
    } else {
        sdClose(openFile);
        return 0;    
    }
}

/*
[desc]	Closes a file.
    
[file] A file to close.
	
[ret]	Returns 1 for success, 0 otherwise.
*/
uint8 sdClose(FS_FILE* file) {
    if(!FS_FClose(file)) {
        return 1;    
    }
    return 0;
}


//EOF
