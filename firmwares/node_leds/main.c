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

#ifndef BROKER_ADDR
#define BROKER_ADDR "2001:660:3207:102::4"
#endif

#ifndef BROKER_PORT
#define BROKER_PORT 8888
#endif

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
static const char * broker_addr = BROKER_ADDR;
static uint16_t broker_port = BROKER_PORT;

static void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);

    char message[5] = "Alive";

    /* format destination address from string */
    ipv6_addr_t dst_addr;
    if (ipv6_addr_from_str(&dst_addr, broker_addr) == NULL) {
        printf("Error: address not valid '%s'\n", broker_addr);
        return NULL;
    }

    for(;;) {
    /* send data to server */
    if (conn_udp_sendto(message, strlen(message), NULL, 0, (struct sockaddr *)&dst_addr,
                sizeof(dst_addr), AF_INET6, (uint16_t)0, broker_port) < 0) {
        printf("Error: couldn't send message to broker.\n");
    }

    /* wait 3 seconds */
    xtimer_usleep(INTERVAL);
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
