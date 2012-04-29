//=============================================================================
// Project:      SIMpad
//=============================================================================
// FILE-NAME:    main.cpp
// FUNCTION:     Serial download of a new image from PC to SIMpad
//
// AUTHOR:       Juergen Messerer, Peter Voser
// CREAT.-DATE:  01.04.2001 (dd.mm.yy)
//
// NOTES:        -
//
//=============================================================================

#include <unistd.h>
#include <iostream>
#include <string.h>
#include "serialdownload.hpp"
using namespace std;

const int STRING_LENGTH = 128;

//=============================================================================
//=============================================================================
void printHelp(void)
{
    cout << "serload V1.0 for Linux " << endl;
    cout << "Downloading a new image to the SIMpad " 
         << "using the serial interface." << endl << endl;
    cout << "Invocation: serload [IMAGEFILE] [ttyS-PORT] [slowbaud]" << endl;
    cout << "IMAGEFILE: the file with the new image prepared for the"
         << " SIMpad Bootloader." 
         << endl;
    cout << "ttyS-PORT: number of the ttyS-Port for the download." 
         << endl;
    cout << "Note: ttyS0 = COM1, ttyS1 = COM2, ..." << endl;
    cout << "For download with 38400 baud give parameter slowbaud"
         << endl;
    cout << "Example: download file: gpe_26.img over COM1 = ttyS[0] with slowbaud (38400)"
         << endl;
    cout << "         serload gpe_26.img 0 slowbaud"
         << endl << endl;
}

//=============================================================================
//=============================================================================
int main(int argc, char *argv[])
{
    int i;
    for(i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "--help") == 0)
        {
	    // The user asks for help.
	    printHelp(); 
            exit(0);
	}
    }

    if(argc !=4 && argc != 3 && argc != 2)
    {
        cerr << endl << "Usage: serload [IMAGEFILE] [ttyS-PORT] [slowbaud]" << endl;
        cerr << "See also \"serload --help\"!" << endl << endl;
        exit(1);
    }
    
    int newBaud;
    newBaud = 115200;
    char device[STRING_LENGTH];
    char test[STRING_LENGTH];
        
    cout << "argc=" << argc << endl;

    if(argc == 4)
    {
        strcpy(test, argv[3]);
        if(test == "slowbaud")
	{
	    newBaud = 38400;
            strcpy(device, "/dev/ttyS");
            strcat(device, argv[2]);	    
	}
	else
	{
            cerr << endl << "Usage: serload [IMAGEFILE] [ttyS-PORT] [slowbaud]" << endl;
            cerr << "See also \"serload --help\"!" << endl << endl;
            exit(1);	
	}
    }

    if(argc == 3)
    {
        cout << "argv[0]=" << argv[0] << endl; 
        cout << "argv[1]=" << argv[1] << endl; 
        cout << "argv[2]=" << argv[2] << endl; 
        if(argv[2] == test)
	{
            cout << "argv[2]=" << argv[2] << endl; 
	    newBaud = 38400;
            // No serial port is given, use ttyS0 as default.
            strcpy(device, "/dev/ttyS0");
	}
	else
	{
            strcpy(device, "/dev/ttyS");
            strcat(device, argv[2]);
	}
    }	        
    else
    {
        // If no serial port is given, use ttyS0 as default.
        strcpy(device, "/dev/ttyS0");
    }

    SerialDownload serload;
    int myError, imagesize;
    static char *image;

    int success = serload.openSerialPort(device, myError);
    if(success != 0)
    {
        cerr << "Error: cannot open " << device << ". Aborting." 
             << endl << endl;
        exit(2);
    }

    myError = serload.loadFile(argv[1], image, imagesize);
    if(myError != 0)
    {
        cerr << "Error: cannot load file " 
             << argv[1] << "! Aborting." << endl << endl;
        exit(3);
    }

    cout << "Please press RESET at the back of the SIMpad!" << endl << endl;
    // int reply = serload.connectToSimpad(115200, myError);

    cout << "Using Baudrate=" << newBaud << " for download" << endl;
    cout << endl << "Try to connect to Simpad over " << device << " ..." << endl;
    int reply = serload.connectToSimpad(newBaud, myError);

    if(reply != 0)
    {
        cerr << "Error: cannot connect to SIMpad! Aborting." 
             << endl << endl;
        exit(4);
    }
    cout << "Connect to Simpad success" << endl << endl;

    // Determine number of blocks to send without remaining bytes!
    int progress = 0;
    int size = imagesize;
    int totalBlocks = size / 512;
    int numberOfBlocksToSend = totalBlocks;
    int numberOfBlocksSent = 0;
    // int count = 0;

    cout << "Start of download ..." << endl << endl;

    // Send blocks.
    while(numberOfBlocksToSend)
    {
/*
        if(count < 64)
        {
            cout << "#";
        }
        else
        {
            cout << "   " << progress << " %" << endl;
            count = 0;
        }
*/
        serload.sendBlock(image, 512, myError);
        image += 512;
        --numberOfBlocksToSend;
        // Update progress info every 100th block.
        if(!(numberOfBlocksSent % 100))
        {
            progress = 100 * numberOfBlocksSent / totalBlocks;
        }
        ++numberOfBlocksSent;
        // ++count;
    }

    // Determine, if there are remaining bytes.
    int numberOfRemainingBytes = size % 512;
    if(numberOfRemainingBytes)
    {
        serload.sendBlock(image, numberOfRemainingBytes, myError);
        // cout << "#   100 %" << endl;
    }

    cout << endl << "Wait for end of burning ..." << endl;
    // The bootloader burns the new image.
    serload.waitForEndOfBurning();
    cout << "Burning done !" << endl << endl;

    cout << "Update successfully finished! Swich the SIMpad on." << endl;

    return 0;
}
