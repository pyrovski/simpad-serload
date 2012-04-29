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
// Updated:      Peter Bailey (peter.eldridge.bailey@gmail.com)
// Date:         April 2012
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
    cout << "serload V1.1 for Linux " << endl;
    cout << "Downloading a new image to the SIMpad " 
         << "using the serial interface." << endl << endl;
    cout << "Invocation: serload [IMAGEFILE] -p [ttyS-PORT] -b [baud]" << endl;
    cout << "IMAGEFILE: the file with the new image prepared for the"
         << " SIMpad Bootloader." 
         << endl;
    cout << "ttyS-PORT: device file of the ttyS-Port for the download." 
         << endl;
    cout << "Note: /dev/ttyS0 = COM1, /dev/ttyS1 = COM2, ..." << endl;
    cout << "For download with 38400 baud give parameter -b 38400"
         << endl;
    cout << "Example: download file: gpe_26.img over COM1 = ttyS[0] with slowbaud (38400)"
         << endl;
    cout << "         ./serload gpe_26.img -p /dev/ttyS0 -b 38400"
         << endl << endl;
}

//=============================================================================
//=============================================================================
int main(int argc, char *argv[])
{

  int status;
  const char optstring[] = "hp:b:";

  int newBaud = 115200;
  char device[STRING_LENGTH] = "/dev/ttyS0";
  

  while((status = getopt(argc, argv, optstring)) != -1){
    switch(status){
    case 'h':
      printHelp();
      exit(0);
    case 'p':
      strncpy(device, optarg, STRING_LENGTH);
      device[STRING_LENGTH - 1] = 0;
      break;
    case 'b':
      newBaud = strtoul(optarg, 0, 0);
      break;
    }
  }

  if(optind >= argc){
    fprintf(stderr, "Expected input file\n");
    printHelp();
    exit(1);
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
