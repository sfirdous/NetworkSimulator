#include <stdio.h>
#include <time.h>
#include "fun.h"

int main()
{
    // Buffer port;

    // // clearBuffers(port);
    // // fillOutBuffer(port,10);
    // // printBuffer("Out",port,0);
    // // memcpy(port[1],port[0],port[0][0]+1);
    // // printBuffer("In",port,1);

    // memset(port,0,BUFFER_SIZE);
    // unsigned char data[] = {0x11, 0x22, 0x33, 0x44};


    // if(createPacket(port,'S',7,4,PKT_DATA,data,sizeof(data))){
    //     printf("Packet created sucessfully.\n");
    //     printPacket(port,0);
    // }else{
    //     printf("Failed to create packet: payload too large,\n");
    // }
    srand(time(NULL));

    int num_hosts;
    printf("Enter number of hosts (max %d): ",MAX_HOSTS);
    scanf("%d",&num_hosts);

    if(num_hosts > MAX_HOSTS) num_hosts = MAX_HOSTS;

    Host hosts[num_hosts];

    initializeHosts(hosts,num_hosts);

    // Simulation loop 
    for(int iter = 0; iter < 5;++iter)
    {
        printf("Simulation Iteration %d\n",iter);

        // Fills out-buffers
        for (int i = 0; i < num_hosts; i++)
            TestPStrip(&hosts[i],num_hosts);
        
        // Print out-buffers
        for (int i = 0; i < num_hosts; i++){
            printf("%c: ",hosts[i].mac);
            printBuffer("Out",hosts[i].buf,0);}
        
        // LAN transfer
        lan_connector(hosts,num_hosts);

        // Print in-buffers after LAN
        for (int i = 0; i < num_hosts; i++){
            printf("%c: ",hosts[i].mac);
            printBuffer("In",hosts[i].buf,1);}
       
        
        //physicalLayerTransfer(hosts,num_hosts);
    }
    
    
   return 0;
}