#include <stdio.h>
#include <time.h>
#include "fun.h"

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

    const int SCHED_ITERS = 10;

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

        // /* 4. Print snapshot (after LAN, after processing) */
        // printf("Snapshot after LAN connector:\n");
        // for (int i = 0; i < num_hosts; ++i)
        // {
        //     printf("%c OUT: ", hosts[i].mac);
        //     printBuffer("Out", hosts[i].buf, 0);
        //     printf("%c IN:  ", hosts[i].mac);
        //     printBuffer("In", hosts[i].buf, 1);
        // }
    }

    /* 5. Print summary stats */
    printf("\n=== Simulation Summary ===\n");
    for (int i = 0; i < num_hosts; ++i)
        printf("Host %c: Sent=%d  Received=%d\n",
               hosts[i].mac, hosts[i].sent, hosts[i].received);

    return 0;
}
