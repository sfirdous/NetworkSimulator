#include <stdio.h>
#include <time.h>
#include "fun.h"

int collision_count = 0; // global counter for LAN collisions

int main()
{
    srand(time(NULL));

    int num_hosts;
    printf("Enter number of hosts (max %d): ", MAX_HOSTS);
    scanf("%d", &num_hosts);
    if (num_hosts > MAX_HOSTS)
        num_hosts = MAX_HOSTS;

    Host hosts[num_hosts];
    initializeHosts(hosts, num_hosts);

    const int SCHED_ITERS = 60;

    // Simulation loop
    for (int iter = 0; iter < SCHED_ITERS; ++iter)
    {
        printf("\n=== Simulation Iteration %d ===\n", iter + 1);

        /* 1. Round robin: each host runs its TestP strip */
        for (int i = 0; i < num_hosts; i++)
            TestPStrip(&hosts[i], num_hosts);

        /* 2. LAN connector simulates the shared medium */
        lan_connector(hosts, num_hosts);

        /* 3. Process incoming frames for each host */
        for (int i = 0; i < num_hosts; i++)
        {
            if (hosts[i].buf[1][0] != 0)
            {
                if (dataLinkLayerReceive(&hosts[i]))
                    networkLayerReceive(&hosts[i]);
                else
                    clearBuffers(&hosts[i], 1); // not for this host
            }
        }

    }

    /* 5. Print summary stats */
    printf("\n=== Simulation Summary ===\n");
    for (int i = 0; i < num_hosts; ++i)
        printf("Host %c: Sent=%d  Received=%d\n", hosts[i].mac, hosts[i].sent, hosts[i].received);

    printf("Total LAN Collisions Detected: %d\n", collision_count);

    printf("\n=== ARP Tables ===\n");
        for (int i = 0; i < num_hosts; ++i) printARPTable(&hosts[i]);


    return 0;
}
