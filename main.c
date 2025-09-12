#include "param.h"
#include "fun.h"

int main()
{
    Buffer port;

    // clearBuffers(port);
    // fillOutBuffer(port,10);
    // printBuffer("Out",port,0);
    // memcpy(port[1],port[0],port[0][0]+1);
    // printBuffer("In",port,1);

    memset(port,0,BUFFER_SIZE);
    unsigned char data[] = {0x11, 0x22, 0x33, 0x44};


    if(createPacket(port,'S',7,4,PKT_DATA,data,sizeof(data))){
        printf("Packet created sucessfully.\n");
        printPacket(port,0);
    }else{
        printf("Failed to create packet: payload too large,\n");
    }

    
   return 0;
}