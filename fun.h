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
int createSIPPacket(Buffer buf,unsigned char net,unsigned char machine,char pkt_type,unsigned char* payload,int payload_len); 
void printPacket(Buffer buf,int index);                         // Function to print packets's details from any buffer index 
void wrapMACFrame(Buffer buf,char dest_mac,char src_mac);       // Wrap SIP packet in MAC frame at Data Link layer

int prob(int percentage);
void checkAndPrintInBuffer(Buffer port,int hostNumber);
void sendRandomPacket(Buffer port,int hostNumber,int numHosts);

// Prases the MAC frame at the Data Link Layer 
// Verifies if the packet is addressed to this host (self_mac)
// Returns 1 if the packet is for this host and passes up to network layer, else 0
int dataLinkLayerReceive(Buffer buf,char self_mac);

// Parses the SIP packet inside buf at index 0 and prints details
// Returns 1 if successful, 0 if packet is invalid
int networkLayerReceive(Buffer buf);


// TestP strip simulating a host with MAC address self_mac on LAN
void TestPStrip(Buffer buf,char self_mac,unsigned char net,unsigned char machine);
#endif