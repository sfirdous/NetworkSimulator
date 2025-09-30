#include "fun.h"
void clearBuffers(Host *host, int index)
{
    host->buf[index][0] = 0; // Clear in/out-buffer byte count
}

int fillOutBuffer(Buffer buf, int byteCount)
{
    if (byteCount < 1 || byteCount >= BUFFER_SIZE)
        return 0;

    buf[0][0] = byteCount; // Set byte count

    for (int i = 1; i <= byteCount; ++i)
        buf[0][i] = rand() % BUFFER_SIZE; // Random data

    return byteCount;
}

void printBuffer(const char *name, Buffer buf, int index) {
    int count = buf[0];
    if (count == 0) {
        printf("%s buffer is empty.\n", name);
        return;
    }
    printf("%s buffer has %d bytes: ", name, count);
    for (int i = 1; i <= count; ++i)
        printf("%02X ", buf[i]);
    printf("\n");
}

int createSIPPacket(Host *host, char pkt_type, unsigned char *payload, int payload_len)
{
    if (payload_len > BUFFER_SIZE - 6)
        return 0; // 6 bytes for header + byte count

    int total_len = 4 + payload_len; // byte count + net + machine + type + payload

    host->buf[0][0] = total_len;               // byte count
    host->buf[0][1] = host->net;               // SIP network byte
    host->buf[0][2] = host->machine;           // SIP machine byte
    host->buf[0][3] = (unsigned char)pkt_type; // Packet type

    // Copy payload bytes
    memcpy(&host->buf[0][4], payload, payload_len);

    return 1; // success
}

void wrapMACFrame(Host *host, char dest_mac)
{
    int sip_len = host->buf[0][0]; // Length of SIP packet

    // Shift SIP packet bytes 2 positions to right to make place for MAC header
    for (int i = sip_len; i >= 1; --i)
    {
        host->buf[0][i + 2] = host->buf[0][i];
    }

    host->buf[0][0] = sip_len + 2; // Update total length

    // Insert MAC addresses in header
    host->buf[0][1] = dest_mac;
    host->buf[0][2] = host->mac;
}

int dataLinkLayerReceive(Host *host)
{
    int frame_len = host->buf[0][0];
    if (frame_len < 6)
        return 0; // Frame too short to cotain MAC+SIP header

    char dest_mac = host->buf[0][1];

    if (dest_mac != host->mac && dest_mac != BROADCAST_MAC)
        return 0;

    // Shift SIP packet bytes to remove MAC header for network layer processing
    for (int i = 3; i < frame_len; ++i)
        host->buf[0][i - 2] = host->buf[0][i];

    // Update packet length in buffer
    host->buf[0][0] = frame_len - 2;

    return 1; // Packet accepted and ready for network layer
}

int networkLayerReceive(Host *host)
{
    int pkt_len = host->buf[0][0];
    if (pkt_len < 4)
        return 0; // Packet too short to be valid SIP packet

    unsigned char net = host->buf[0][1];
    unsigned char machine = host->buf[0][2];
    char pkt_type = (char)host->buf[0][3];

    int payload_len = pkt_len - 4; // Payload length
    unsigned char *payload = &host->buf[0][4];

    // Print packet info per spec or requirments
    printf("Received SIP Packet: Net=%d, MAchine=%d, Type=%d, PayloadLen=%d\n", net, machine, pkt_type, payload_len);

    for (int i = 0; i < payload_len; ++i)
        printf("%02X", payload[i]);

    printf("\n");

    return 1; // Sucessfully parsed and printed
}

void printPacket(Buffer buf, int index)
{
    int len = buf[index][0];
    if (len == 0)
    {
        printf("Buffer is empty.\n");
        return;
    }

    char mac = (char)buf[index][1];
    unsigned char net = buf[index][2];
    unsigned char machine = buf[index][3];
    char pkt_type = (char)buf[index][4];
    int payload_len = len - 5;

    printf("Packet details:\n");
    printf(" Byte Count: %d\n", len);
    printf(" MAC address: %c\n", mac);
    printf(" SIP: (%d, %d)\n", net, machine);
    printf(" Packet type: %c\n", pkt_type);

    printf(" Payload (%d bytes): ", payload_len);
    for (int i = 0; i < payload_len; ++i)
    {
        printf("%02X ", buf[index][i]);
    }
    printf("\n");
}

int prob(int percentage)
{
    return ((rand() % 100) < percentage); // returns 1 if event occurs, else 0
}

void checkAndPrintInBuffer(Buffer buf, int hostNumber)
{
    int len = buf[1][0];
    if (len != 0)
    {
        // Print message in required format
        printf("TestP%d received: ", hostNumber);
        printBuffer("In", buf, 1);
    }
    buf[1][0] = 0; // Clear buffer after processing
}

// void sendRandomPacket(Buffer port,int hostNumber,int numHosts){
//     if(!prob(10)) return;                 // Only 10% chance to send

//     unsigned char payload[20];
//     int payload_len = 13 + (rand() % 8);  // Random size: 13-20 bytes

//     for(int i = 0 ; i < payload_len ; ++i) payload[i] = rand() % 256;

//     // char destHost = 'A' + (rand()% numHosts);       //
// }

void physicalLayerSend(Buffer buf)
{
    int pkt_len = buf[0][0];
    printf("Physical layer send: Packet length %d bytes\n", pkt_len);
}

void TestPStrip(Host *host, int num_hosts)
{

    // 1. Receive and process incoming packets
    if (dataLinkLayerReceive(host))
    {                              // Check and accept packets addressed to self_mac
        networkLayerReceive(host); // Parse and print SIP packet
        clearBuffers(host, 1);     // Clear in-buffer after processig
    }

    // 2. With 10% probability, create and send a new packet
    if (prob(10))
    {
        int payload_len = 13 + (rand() % 8);
        unsigned char payload[payload_len];

        // Fill payload with random bytes
        for (int i = 0; i < payload_len; ++i)
            payload[i] = (unsigned char)(rand() % 256);

        // Choose random destination MAC different from sender
        char dest_mac;
        do
        {
            dest_mac = 'A' + rand() % num_hosts;
        } while (dest_mac == host->mac);

        // Create SIP Packet
        if (createSIPPacket(host, 'D', payload, payload_len))
        {
            //  Wrap in MAC  frame
            wrapMACFrame(host, dest_mac);

            // Send packet at physical layer (implement next)
            physicalLayerSend(host->buf);
            printPacket(host->buf, 0);
        }
    }
}

void physicalLayerTransfer(Host host[], int num_hosts)
{
    for (int i = 0; i < num_hosts; ++i)
    {
        int pkt_len = host[i].buf[0][0];
        if (pkt_len < 6)
            continue; // no valid frame

        char dest_mac = host[i].buf[0][1];
        int dest_index = dest_mac - 'A';
        if (dest_index < 0 || dest_index >= num_hosts)
            continue; // invalid dest

        // Copy ou-buffer to destination's in-buffer
        for (int j = 0; j <= pkt_len; ++j)
            host[dest_index].buf[1][j] = host[i].buf[0][j];

        // Clear senders's out-buffer after sending
        clearBuffers(&host[i], 0);
    }
}

void initializeHosts(Host hosts[], int num_hosts)
{
    for (int i = 0; i < num_hosts; ++i)
    {
        hosts[i].mac = 'A' + i;
        hosts[i].net = 1;           // Static net
        hosts[i].machine = i + 1;   // Unique machine number
        clearBuffers(&hosts[i], 0); // Clear out-buffer
        clearBuffers(&hosts[i], 1); // Clear in-buffer
        printf("Host %c initialized: Network %d, Machine %d\n", hosts[i].mac, hosts[i].net, hosts[i].machine);
    }
}

void lan_connector(Host *hosts, int numhosts)
{
    int transmitting_hosts = 0;
    int sender_index = -1;

    // Count how many hosts have packets to trasmit in out-buffer
    for (int i = 0; i < numhosts; ++i)
    {
        if (hosts[i].buf[0][0] > 0)
        {
            transmitting_hosts++;
            sender_index = i;
        }

        if (transmitting_hosts == 0)
        {
            // No transmissions, do nothing
            return;
        }
        else if (transmitting_hosts > 1)
        {
            // Collision detected, drop all packets from out-buffers
            printf("LAN collisin detected! Dropping all outgoing packets this round.\n");
            for (int i = 0; i < numhosts; i++)
            {
                hosts[i].buf[0][0] = 0;
            }
        }
        else
        {
            for(int i = 0; i < numhosts;++i){
                if(i != sender_index){
                    memcpy(hosts[i].buf[1],hosts[sender_index].buf[0],BUFFER_SIZE);
                }
                hosts[sender_index].buf[0][0] = 0;
            }
            printf("Host %c broadcast its packet to all other hosts.\n",hosts[sender_index].mac);
        }
    }
}