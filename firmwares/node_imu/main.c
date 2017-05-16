/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdlib.h>
#include <string.h>

#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "board.h"
#include "nanocoap.h"
#include "net/gcoap.h"

/* RIOT firmware libraries */
#include "coap_common.h"
#include "coap_utils.h"
#include "coap_imu.h"

#ifndef BROKER_ADDR
#define BROKER_ADDR "2001:660:3207:102::4"
#endif

#define BROKER_PORT 5683

#define INTERVAL              (30000000U)    /* set interval to 30 seconds */
#define IMU_INTERVAL          (200000U)      /* set imu refresh interval to 200 ms */
#define MAIN_QUEUE_SIZE       (8)
#define BEACONING_QUEUE_SIZE  (8)
#define IMU_QUEUE_SIZE  (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static msg_t _beaconing_msg_queue[BEACONING_QUEUE_SIZE];
static char beaconing_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t _imu_msg_queue[IMU_QUEUE_SIZE];
static char imu_stack[THREAD_STACKSIZE_DEFAULT];

static uint8_t payload[256] = { 0 };
static uint8_t response[256] = { 0 };

/* broker  */
static const char * broker_addr = BROKER_ADDR;

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/board", COAP_GET, board_handler },
    { "/imu", COAP_GET, coap_imu_handler },
    { "/mcu", COAP_GET, mcu_handler },
    { "/name", COAP_GET, name_handler },
    { "/os", COAP_GET, os_handler },
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};

void *imu_thread(void *args)
{
    msg_init_queue(_imu_msg_queue, IMU_QUEUE_SIZE);
    
    for(;;) {
        size_t p = 0;
        read_imu_values(payload);
        p += sprintf((char*)&response[p], "imu:");
        p += sprintf((char*)&response[p], (char*)payload);
        response[p] = '\0';
        printf("Sending %s\n",response);
        send_coap_post(broker_addr, BROKER_PORT, (uint8_t*)"server", response);
        /* wait 3 seconds */
        xtimer_usleep(IMU_INTERVAL);
    }
    return NULL;
}


void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);
    
    for(;;) {
        printf("Sending Alive\n");
        send_coap_post(broker_addr, BROKER_PORT, (uint8_t*)"alive", (uint8_t*)"Alive");
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
    _netif_config(0, NULL);;

    /* start coap server loop */
    gcoap_register_listener(&_listener);

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
    
    /* create the sensors thread that will send periodic updates to
       the server */
    int imu_pid = thread_create(imu_stack, sizeof(imu_stack),
                                THREAD_PRIORITY_MAIN - 1,
                                THREAD_CREATE_STACKTEST, imu_thread,
                                NULL, "IMU thread");
    if (imu_pid == -EINVAL || imu_pid == -EOVERFLOW) {
        puts("Error: failed to create imu thread, exiting\n");
    }
    else {
        puts("Successfuly created imu thread !\n");
    }

#ifdef LED0_TOGGLE
    LED0_TOGGLE;
#endif
#ifdef LED0_TOGGLE
    LED1_TOGGLE;
#endif
#ifdef LED0_TOGGLE
    LED2_TOGGLE;
#endif

    /* should be never reached */
    return 0;
}
