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


int createPacket(Buffer buf,char mac,unsigned char net,unsigned char machine,char pkt_type,unsigned char* payload,int payload_len){
    if(payload_len > BUFFER_SIZE - 5)  return 0;    // 5 bytes for header + byte count

    int total_len = 5+payload_len;                  // byte count + mac + net + machine + type + payload

    buf[0][0] = total_len;                          // byte count
    buf[0][1] = (unsigned char)mac;                 // MAC address
    buf[0][2] = net;                                // SIP network byte
    buf[0][3] = machine;                            // SIP machine byte
    buf[0][4] = (unsigned char)pkt_type;            // Packet type

    // Copy payload bytes
    memcpy(&buf[0][5],payload,payload_len);

    return 1; // success

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
