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