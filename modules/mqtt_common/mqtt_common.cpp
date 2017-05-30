#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "thread.h"
#include "xtimer.h"

#include "mqtt_common.h"
#include "mqtt_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef APPLICATION_NAME
#define APPLICATION_NAME "Node"
#endif

#define BEACON_INTERVAL       (30000000U)    /* set interval to 30 seconds */

#define BEACONING_QUEUE_SIZE  (8U)
static msg_t _beaconing_msg_queue[BEACONING_QUEUE_SIZE];
static char beaconing_stack[THREAD_STACKSIZE_DEFAULT];

const char * app_name = APPLICATION_NAME;

const char * node_id = NODE_ID;

void get_board(char *value) {
    sprintf(value, "%s", RIOT_BOARD);
}

void get_mcu(char *value) {
    sprintf(value, "%s", RIOT_MCU);
}

void get_os(char *value) {
    sprintf(value, "riot");
}

void get_name(char *value) {
    sprintf(value, app_name);
}

void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);
    for(;;) {
        publish((uint8_t*)"node/check", (uint8_t*)node_id);
        /* wait 30 seconds */
        xtimer_usleep(BEACON_INTERVAL);
    }
    return NULL;
}

void init_beacon_sendor(void)
{
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
}
