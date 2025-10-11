#ifndef FUN_H
#define FUN_H

#include "param.h"

extern int collision_count;

/* utilities */
void printBuffer(const char* name,Buffer buf,int index);  // Function to print contents of both buffers
int prob(int percentage);
void clearBuffers(Host *host,int index);                  // Function to clear both in and out buffers


/*Packet creation/encapsulation */
int createSIPPacket(Host* host,char pkt_type,unsigned char* payload,int payload_len); 
int wrapMACFrame(Host * host,char dest_mac);       // Wrap SIP packet in MAC frame at Data Link layer

/* Layers */
int dataLinkLayerReceive(Host* host);
// Parses the SIP packet inside buf at index 0 and prints details
// Returns 1 if successful, 0 if packet is invalid
int networkLayerReceive(Host *host);

/* printing/debugging*/
void printPacket(Buffer buf,int index);                         // Function to print packets's details from any buffer index 

/* Test strip and conectors*/
// TestP strip simulating a host with MAC address self_mac on LAN
void TestPStrip(Host *host,int num_hosts);
void lan_connector(Host *hosts,int numhosts);

/*physical layer*/
void physicalLayerSend(Buffer buf);
// Simulate physical layer: move out-buffer from sender to dest's in-buffer
void physicalLayerTransfer(Host host[],int num_hosts);

/* initialization*/
// Initialize hosts with MAC,network,machine IDS and clear buffers
void initializeHosts(Host hosts[],int num_hosts);

char lookupARP(Host *host,unsigned char net,unsigned char machine);
void updateARP(Host *host,unsigned char net,unsigned char machine,char mac);
void printARPTable(Host *host);

#endif