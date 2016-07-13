/*
 * Copyright (C) 2016 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       CoAP example server application (using microcoap)
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <stdio.h>
#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "bmp180.h"
#include "board.h"
#include "net/af.h"
#include "net/gnrc/ipv6.h"
#include "net/conn/udp.h"

#define INTERVAL              (30000000U)    /* set interval to 30 seconds */
#define MAIN_QUEUE_SIZE       (8)
#define BEACONING_QUEUE_SIZE  (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static msg_t _beaconing_msg_queue[BEACONING_QUEUE_SIZE];
static char beaconing_stack[THREAD_STACKSIZE_DEFAULT];

void microcoap_server_loop(void);

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

/* broker  */
static const ipv6_addr_t broker_addr = {{ 0x20, 0xa1, 0x04, 0xf8, \
                                          0x0c, 0x17, 0x3f, 0xd8, \
                                          0x00, 0x00, 0x00, 0x00, \
                                          0x00, 0x00, 0x00, 0x02 }};
static uint16_t BROKER_PORT = 8888;

static void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);
    
    uint32_t last_wakeup = xtimer_now();
    char message[5] = "Alive";
    
    for(;;) {
	/* send data to server */
	if (conn_udp_sendto(message, strlen(message), NULL, 0, (struct sockaddr *)&broker_addr,
			    sizeof(broker_addr), AF_INET6, (uint16_t)0, BROKER_PORT) < 0) {
	    printf("Error: couldn't send message to broker.\n");
	}

	/* wait 5 seconds */
	xtimer_usleep_until(&last_wakeup, INTERVAL);
    }

    return NULL;
}

int main(void)
{   
    puts("RIOT microcoap example application");

    /* microcoap_server uses conn which uses gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    
    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _netif_config(0, NULL);

    /* create the beaconning thread that will send periodic messages to 
       the broker */
    int beacon_pid = thread_create(beaconing_stack, sizeof(beaconing_stack),
                                   THREAD_PRIORITY_MAIN - 1,
				   THREAD_CREATE_STACKTEST, beaconing_thread,
                                   NULL, "Beaconing thread");
    if (beacon_pid == -EINVAL || beacon_pid == -EOVERFLOW) {
	puts("Error: failed to create beaconing thread, exiting\n");
    }
    else {
	puts("Successfuly created beaconing thread !\n");
    }
    
    /* start coap server loop */
    microcoap_server_loop();

    /* should be never reached */
    return 0;
}
