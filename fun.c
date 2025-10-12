#include "fun.h"
void clearBuffers(Host *host, int index)
{
    host->buf[index][0] = 0; // Clear in/out-buffer byte count
}

int prob(int percentage)
{
    const int N = 20000;
    int k = rand() % N;
    int j = (N * percentage) / 100;
    return (k < j); // returns 1 with probability percetange%
}

void printBuffer(const char *name, Buffer buf, int index)
{
    int count = buf[index][0];
    if (count == 0)
    {
        printf("%s buffer is empty.\n", name);
        return;
    }
    printf("%s buffer has %d bytes: ", name, count);
    for (int i = 1; i <= count; ++i)
        printf("%02X ", buf[index][i]);
    printf("\n");
}

int createSIPPacket(Host *host, char pkt_type, unsigned char *payload, int payload_len)
{
    if (payload_len < 0 || payload_len > BUFFER_SIZE - 6)
        return 0; // 6 bytes for header + byte count

    int total_len = 4 + payload_len; // byte count + net + machine + type + payload

    if (total_len >= BUFFER_SIZE)
        return 0;

    host->buf[0][0] = total_len;               // byte count
    host->buf[0][1] = host->net;               // SIP network byte
    host->buf[0][2] = host->machine;           // SIP machine byte
    host->buf[0][3] = (unsigned char)pkt_type; // Packet type

    // Copy payload bytes
    memcpy(&host->buf[0][4], payload, payload_len);

    return 1; // success
}

int wrapMACFrame(Host *host, char dest_mac)
{
    int sip_len = host->buf[0][0]; // Length of SIP packet
    if (sip_len + 2 >= BUFFER_SIZE)
        return 0;

    // Shift SIP packet bytes 2 positions to right to make place for MAC header
    for (int i = sip_len; i >= 1; --i)
        host->buf[0][i + 2] = host->buf[0][i];

    host->buf[0][0] = sip_len + 2; // Update total length

    // Insert MAC addresses in header
    host->buf[0][1] = dest_mac;
    host->buf[0][2] = host->mac;

    return 1;
}

int dataLinkLayerReceive(Host *host)
{
    int frame_len = host->buf[1][0];
    if (frame_len < 6)
        return 0; // Frame too short to cotain MAC+SIP header

    char dest_mac = host->buf[1][1];

    if (dest_mac != host->mac && dest_mac != BROADCAST_MAC)
    {
        host->buf[1][0] = 0;
        return 0;
    }
    // Shift SIP packet bytes to remove MAC header for network layer processing
    int new_len = frame_len - 2;
    for (int i = 1; i <= new_len; ++i)
        host->buf[0][i] = host->buf[1][i + 2];

    // Update packet length in buffer
    host->buf[0][0] = new_len;

    // clear in-buffer
    host->buf[1][0] = 0;

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

    // if(payload_len < 3) return 0;

    // Print packet info per spec or requirments
    printf("Host %c: Received SIP packet -> SIP=(%d,%d) Type=%c PayloadLen=%d Data=",
           host->mac, net, machine, pkt_type, payload_len);

    for (int i = 0; i < payload_len; ++i)
        printf("%02X ", payload[i]);

    printf("\n");

    host->received += 1;

    if (pkt_type == PKT_ARP_REQ)
    {
        if (payload_len < 2) return 0;
        unsigned char req_net = payload[0];
        unsigned char req_machine = payload[1];

        if (req_net == host->net && req_machine == host->machine)
        {
            printf("Host %c: Received ARP Request from (%d,%d). Sendig ARP Reply.\n", host->mac, net, machine);

            unsigned char reply_payload[3] = {host->net, host->machine, host->mac};
            if (createSIPPacket(host, PKT_ARP_REPLY, reply_payload, 3))
            {
                char dest_mac = 'A' + (machine - 1);
                wrapMACFrame(host, dest_mac);
                host->sent++;
                printf("Host %c: Sending ARP Reply to Host %c.\n", host->mac, dest_mac);
            }
        }
    }

    else if (pkt_type == PKT_ARP_REPLY)
    {
        if(payload_len < 3) return 0;
        unsigned char reply_net = payload[0];
        unsigned char reply_machine = payload[1];

        char reply_mac = payload[2];
        printf("Host %c: Received ARP Reply -> (%d,%d) is at MAC = %c\n", host->mac, reply_net, reply_machine, reply_mac);

        updateARP(host, reply_net, reply_machine, reply_mac);
    }

    if (pkt_type == PKT_DATA)
    {
        // Try to find destination MAC for the sender
        char dest_mac = lookupARP(host, net, machine);
        if (dest_mac == 0)
        {
            printf("Host %c: Unknown MAC for (%d,%d). Sending ARP Request.\n",
                   host->mac, net, machine);

            // Create ARP Request payload: [target_net, target_machine]
            unsigned char arp_payload[2] = {net, machine};

            // Build and broadcast ARP Request
            if (createSIPPacket(host, PKT_ARP_REQ, arp_payload, 2))
            {
                wrapMACFrame(host, BROADCAST_MAC);
                host->sent++;
            }

            // Don’t send ACK yet — wait until ARP Reply arrives
            return 0;
        }

        unsigned char ack_payload[1] = {0xAC};
        if (createSIPPacket(host, 'A', ack_payload, 1))
        {
            wrapMACFrame(host, dest_mac);
            host->sent++;
            printf("Host %c: Sending ACK to Host %c\n", host->mac, dest_mac);
        }
    }

    // after processing, input out buf
    host->buf[1][0] = 0;

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
    // char srcmac = (char)buf[index][2];
    unsigned char net = buf[index][3];
    unsigned char machine = buf[index][4];
    char pkt_type = (char)buf[index][5];
    int payload_len = len - 5;

    printf("Packet details:\n");
    printf(" Byte Count: %d\n", len);
    printf(" MAC address: %c\n", mac);
    printf(" SIP: (%d, %d)\n", net, machine);
    printf(" Packet type: %c\n", pkt_type);

    printf(" Payload (%d bytes): ", payload_len);
    for (int i = 0; i < payload_len; ++i)
    {
        printf("%02X ", buf[index][6 + i]);
    }
    printf("\n");
}

void TestPStrip(Host *host, int num_hosts)
{

    // Dont generate a new message if an ACK or pending frame is waiting
    if (host->buf[0][0] != 0)
        return;

    // 1. Receive and process incoming packets
    if (host->buf[1][0] != 0)
    {
        printf("TestP%c received: ", host->mac);
        printBuffer("In", host->buf, 1);
    }

    // 2. With 10% probability, create and send a new packet
    if (!prob(10 * host->speed / 3)) // faster hosts have higher chace to send
        return;

    int payload_len = 13 + (rand() % 8);
    unsigned char payload[BUFFER_SIZE];

    // Fill payload with random bytes
    for (int i = 0; i < payload_len; ++i)
        payload[i] = (unsigned char)(rand() % 256);

    // Choose random destination MAC different from sender
    int dest_index;
    if (num_hosts <= 1)
        return;
    do
    {
        dest_index = rand() % num_hosts;
    } while ((char)('A' + dest_index) == host->mac);

    unsigned char dest_net = host->net;
    unsigned char dest_machine = dest_index + 1;

    char dest_mac = lookupARP(host, dest_net, dest_machine);

    if (!dest_mac)
    {
        unsigned char arp_payload[2] = {dest_net, dest_machine};
        if (createSIPPacket(host, PKT_ARP_REQ, arp_payload, 2))
        {
            wrapMACFrame(host, BROADCAST_MAC);
            host->sent++;
            printf("Host %c: Unknown MAC for (%d,%d). Sending ARP Request.\n",
                   host->mac, dest_net, dest_machine);
        }
        return;
    }

    // Create SIP Packet
    if (createSIPPacket(host, PKT_DATA, payload, payload_len)) // randomize type of packet
    {
        //  Wrap in MAC  frame
        wrapMACFrame(host, dest_mac);
        host->sent++;

        // human-readable message
        // Send packet at physical layer (implement next)

        unsigned int msgid = 0;
        for (int k = 0; k < payload_len && k < 4; ++k)
            msgid = (msgid << 8) | payload[k];

        printf("TestP%c sent the message %u to TestP%c (payload %d bytes)\n", host->mac, msgid, dest_mac, payload_len);
    }
}

/* LAN connector: given hosts and their LAN ports, act as connector between LAN-1..N and LLL lower ports.
   The simple LAN behavior:
   - Count how many input ports have data
   - If 0: do nothing
   - If >1: collision -> drop all inputs
   - If exactly 1: copy that input frame to all other hosts' in-buffer
*/
void lan_connector(Host *hosts, int numhosts)
{
    int transmitting_hosts = 0;
    int sender_index = -1;

    // Count how many hosts are transmitting
    for (int i = 0; i < numhosts; ++i)
    {
        if (hosts[i].buf[0][0] > 0)
        {
            transmitting_hosts++;
            sender_index = i;
        }
    }

    if (transmitting_hosts == 0)
        return;

    if (transmitting_hosts > 1)
    {
        printf("LAN collision detected! Dropping all outgoing packets this round.\n");
        collision_count++;
        for (int i = 0; i < numhosts; i++)
            hosts[i].buf[0][0] = 0;
        return;
    }

    Host *sender = &hosts[sender_index];
    unsigned char *frame = sender->buf[0];
    int frame_len = frame[0];
    char dest_mac = (char)frame[1];

    if (dest_mac == BROADCAST_MAC)
    {
        printf("Host %c broadcast its packet to all other hosts.\n", sender->mac);

        for (int i = 0; i < numhosts; ++i)
        {
            if (i == sender_index)
                continue;

            int upto = (frame_len < BUFFER_SIZE) ? frame_len : BUFFER_SIZE - 1;
            for (int j = 0; j <= upto; ++j)
                hosts[i].buf[1][j] = frame[j];

            printf("Host %c received broadcast frame from Host %c.\n", hosts[i].mac, sender->mac);
        }
    }
    else // Unicast frame
    {
        for (int i = 0; i < numhosts; ++i)
        {
            if (hosts[i].mac == dest_mac)
            {
                if (hosts[i].buf[1][0] != 0)
                {
                    printf("LAN Warning: Host %c input buffer full! Replacing with new unicast packet from %c.\n",
                           hosts[i].mac, sender->mac);
                }

                int upto = (frame_len < BUFFER_SIZE) ? frame_len : BUFFER_SIZE - 1;
                for (int j = 0; j <= upto; ++j)
                    hosts[i].buf[1][j] = frame[j];

                printf("Host %c sent frame directly to Host %c.\n", sender->mac, hosts[i].mac);
                break;
            }
        }
    }

    // Clear sender’s output buffer
    sender->buf[0][0] = 0;
}


void initializeHosts(Host hosts[], int num_hosts)
{
    for (int i = 0; i < num_hosts; ++i)
    {
        hosts[i].mac = (char)('A' + i);
        hosts[i].net = 1;                          // Static net
        hosts[i].machine = (unsigned char)(i + 1); // Unique machine number
        clearBuffers(&hosts[i], 0);                // Clear out-buffer
        clearBuffers(&hosts[i], 1);                // Clear in-buffer
        hosts[i].sent = 0;
        hosts[i].received = 0;
        hosts[i].speed = 1 + rand() % 5;
        printf("Host %c initialized: Network %d, Machine %d\n", hosts[i].mac, hosts[i].net, hosts[i].machine);

        hosts[i].arp_count = 0;
        for (int j = 0; j < MAX_ARP_ENTRIES; ++j)
        {
            hosts[i].arp_table[j].net = 0;
            hosts[i].arp_table[j].machine = 0;
            hosts[i].arp_table[j].mac = 0;
        }
    }
}

// Look up MAC from ARP table
char lookupARP(Host *host, unsigned char net, unsigned char machine)
{
    for (int i = 0; i < host->arp_count; i++)
    {
        if (host->arp_table[i].net == net && host->arp_table[i].machine == machine)
            return host->arp_table[i].mac;
    }

    return 0;
}

// Add or update entry in ARP table
void updateARP(Host *host, unsigned char net, unsigned char machine, char mac)
{
    for (int i = 0; i < host->arp_count; i++)
    {
        if (host->arp_table[i].net == net && host->arp_table[i].machine == machine)
        {
            host->arp_table[i].mac = mac;
            return;
        }
    }
    if (host->arp_count < MAX_ARP_ENTRIES)
    {
        host->arp_table[host->arp_count].net = net;
        host->arp_table[host->arp_count].machine = machine;
        host->arp_table[host->arp_count].mac = mac;
        host->arp_count++;
    }

}

void printARPTable(Host *host)
{
    printf("ARP Table for Host %c:\n", host->mac);
    for (int i = 0; i < host->arp_count; ++i)
        printf(" (%d,%d) -> %c\n",
               host->arp_table[i].net,
               host->arp_table[i].machine,
               host->arp_table[i].mac);
}