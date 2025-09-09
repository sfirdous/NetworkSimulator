#include "param.h"
#include "fun.h"

int main()
{
    Buffer port;

    clearBuffers(port);

    fillOutBuffer(port,10);
    printBuffer("Out",port,0);

    memcpy(port[1],port[0],port[0][0]+1);
    printBuffer("In",port,1);
    
   return 0;
}