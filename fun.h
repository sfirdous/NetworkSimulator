#ifndef FUN_H
#define FUN_H

#include "param.h"

void clearBuffers(Buffer buf);                            // Function to clear both in and out buffers
int fillOutBuffer(Buffer buf,int byteCount);              // Function to fill out-buffer with random data
void printBuffer(const char* name,Buffer buf,int index);  // Function to print contents of both buffers


// Function to create packet in out-buffer
// mac : MAC address character (A-Z)
// net,machine: SIP address bytes (0-9)
// pkt_type: packet type character
// payload: pointer to payload data bytes
// payload_len : number of payload bytes (max 250)
// Returns 1 if sucessful, 0 if payload too large
int createPacket(Buffer buf,char mac,unsigned char net,unsigned char machine,char pkt_type,unsigned char* payload,int payload_len); 
void printPacket(Buffer buf,int index);                 // Function to print packets's details from any buffer index 

#endif