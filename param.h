#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 256
#define MAX_HOSTS 26
#define BROADCAST_MAC '*'

// Packet types
#define PKT_ACK 'A'
#define PKT_DATA 'D'
#define PKT_BROADCAST 'B'
#define PKT_CONTROL 'C'
#define PKT_ARP_REQ 'L'
#define PKT_ARP_REPLY 'R'
#define MAX_ARP_ENTRIES 10

// Define a port buffer structure d[2][256]
typedef unsigned char Buffer[2][BUFFER_SIZE];

typedef struct{
    unsigned char net;
    unsigned char machine;
    char mac;
}ArpEntry;

typedef struct 
{
    char mac;                           // Host's MAC address
    unsigned char net;                  // Host's Network number
    unsigned char machine;              // Host's Machine number in network
    Buffer buf;                         // Buffer for packets
    int sent;
    int received;
    ArpEntry arp_table[MAX_ARP_ENTRIES];
    int arp_count;
} Host;


