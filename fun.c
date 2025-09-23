#include "fun.h"
void clearBuffers(Buffer buf){
    buf[0][0] = 0;      // Clear out-buffer byte count
    buf[1][0] = 0;      // Clear in-buffer byte count
}

int fillOutBuffer(Buffer buf,int byteCount){
    if(byteCount < 1 || byteCount >= BUFFER_SIZE) return 0;

    buf[0][0] = byteCount; // Set byte count

    for(int i = 1 ; i <= byteCount ; ++i)
        buf[0][i] = rand() % BUFFER_SIZE;  // Random data
    
    return byteCount;
}

void printBuffer(const char* name,Buffer buf,int index){
    int count = buf[index][0];
    if(count == 0)
        printf("%s buffer is empty.\n",name);
    
    printf("%s buffer has %d bytes: ",name,count);
    for(int i = 1;i<=count ; ++i)
        printf("%02X ",buf[index][i]);

    printf("\n");
}


int createSIPPacket(Buffer buf,unsigned char net,unsigned char machine,char pkt_type,unsigned char* payload,int payload_len){
    if(payload_len > BUFFER_SIZE - 6)  return 0;    // 6 bytes for header + byte count

    int total_len = 4+payload_len;                  // byte count + net + machine + type + payload

    buf[0][0] = total_len;                          // byte count
    buf[0][1] = net;                                // SIP network byte
    buf[0][2] = machine;                            // SIP machine byte
    buf[0][3] = (unsigned char)pkt_type;            // Packet type

    // Copy payload bytes
    memcpy(&buf[0][4],payload,payload_len);

    return 1; // success

}

void wrapMACFrame(Buffer buf,char dest_mac,char src_mac)
{
    int sip_len = buf[0][0];                    // Length of SIP packet
    
    // Shift SIP packet bytes 2 positions to right to make place for MAC header
    for(int i = sip_len ; i >= 1 ; --i){
        buf[0][i+2] = buf[0][i];
    }

    buf[0][0] = sip_len+2;                     // Update total length

    // Insert MAC addresses in header
    buf[0][1] = dest_mac;
    buf[0][2] = src_mac;
}

int dataLinkLayerReceive(Buffer buf,char self_mac){
    int frame_len = buf[0][0];
    if(frame_len < 6)
        return 0;                            // Frame too short to cotain MAC+SIP header
    

    char dest_mac = buf[0][1];
    char src_mac = buf[0][2];

    if(dest_mac != self_mac && dest_mac != BROADCAST_MAC)
        return 0;
    
    // Shift SIP packet bytes to remove MAC header for network layer processing
    for(int i = 3;i<frame_len;++i)
        buf[0][i-2] = buf[0][i];
    
    // Update packet length in buffer 
    buf[0][0] = frame_len -2;

    return 1;               // Packet accepted and ready for network layer 
}

int networkLayerReceive(Buffer buf)
{
    int pkt_len = buf[0][0];
    if(pkt_len < 4)
        return 0;                   //Packet too short to be valid SIP packet
    
    unsigned char net = buf[0][1];
    unsigned char machine = buf[0][2];
    char pkt_type = (char)buf[0][3];

    int payload_len = pkt_len - 4;      // Payload length
    unsigned char* payload = &buf[0][4];

    // Print packet info per spec or requirments
    printf("Received SIP Packet: Net=%d, MAchine=%d, Type=%d, PayloadLen=%d\n",net,machine,pkt_type,payload_len);

    for(int i=0;i<payload_len;++i)
        printf("%02X",payload[i]);

    printf("\n");
    
    return 1;       //Sucessfully parsed and printed
    
}

void printPacket(Buffer buf,int index){
    int len = buf[index][0];
    if(len == 0){
        printf("Buffer is empty.\n");
        return;
    }

    char mac = (char)buf[index][1];
    unsigned char net = buf[index][2];
    unsigned char machine = buf[index][3];
    char pkt_type = (char)buf[index][4];
    int payload_len = len - 5;

    printf("Packet details:\n");
    printf(" Byte Count: %d\n",len);
    printf(" MAC address: %c\n",mac);
    printf(" SIP: (%d, %d)\n",net,machine);
    printf(" Packet type: %c\n",pkt_type);

    printf(" Payload (%d bytes): ",payload_len);
    for(int i = 0; i < payload_len ; ++i){
        printf("%02X ",buf[index][i]);
    }
    printf("\n");
}

int prob(int percentage){
    return ((rand() % 100) < percentage);  // returns 1 if event occurs, else 0
}

void checkAndPrintInBuffer(Buffer buf,int hostNumber){
    int len = buf[1][0];
    if(len != 0){
        // Print message in required format
        printf("TestP%d received: ",hostNumber);
        printBuffer("In",buf,1);
    }
    buf[1][0] = 0;                        // Clear buffer after processing
}

void sendRandomPacket(Buffer port,int hostNumber,int numHosts){
    if(!prob(10)) return;                 // Only 10% chance to send

    unsigned char payload[20];
    int payload_len = 13 + (rand() % 8);  // Random size: 13-20 bytes

    for(int i = 0 ; i < payload_len ; ++i) payload[i] = rand() % 256;

    char destHost = 'A' + (rand()% numHosts);       // 
}

void TestPStrip(Buffer buf,char self_mac,unsigned char net,unsigned char machine){

    // 1. Receive and process incoming packets
    if(dataLinkLayerReceive(buf,self_mac)){         // Check and accept packets addressed to self_mac
        networkLayerReceive(buf);                   // Parse and print SIP packet
        clearBuffers(buf);                          // Clear in-buffer after processig
    }

    // 2. With 10% probability, create and send a new packet
    if(prob(10)){
        int payload_len = 13 + (rand() % 8);
        unsigned char payload[payload_len];
        
        // Fill payload with random bytes
        for(int i = 0 ; i < payload_len;++i)
            payload[i] = (unsigned char)(rand() % 256);
        
        // Choose random destination MAC different from sender
        char dest_mac;
        do{
            dest_mac = 'A' + rand() % 26;
        }while(dest_mac == self_mac);

        // Create SIP Packet
        if(createSIPPacket(buf,net,machine,'D',payload,payload_len)){
            //  Wrap in MAC  frame
            wrapMACFrame(buf,dest_mac,self_mac);

            // Send packet at physical layer (implement next)
            physicalLayerSend(buf);
            printPacket(buf,0);
        }
    }
}

