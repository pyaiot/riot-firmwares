#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "nanocoap.h"
#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_utils.h"

#define BEACON_INTERVAL       (30000000U)    /* set interval to 30 seconds */

#define BEACONING_QUEUE_SIZE  (8U)
static msg_t _beaconing_msg_queue[BEACONING_QUEUE_SIZE];
static char beaconing_stack[THREAD_STACKSIZE_DEFAULT];

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(APPLICATION_NAME);
    memcpy(pdu->payload, APPLICATION_NAME, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *board = RIOT_BOARD;
    size_t payload_len = strlen(RIOT_BOARD);
    memcpy(pdu->payload, board, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *mcu = RIOT_MCU;
    size_t payload_len = strlen(RIOT_MCU);
    memcpy(pdu->payload, mcu, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *os = "riot";
    size_t payload_len = strlen("riot");
    memcpy(pdu->payload, os, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);

    for(;;) {
        printf("Sending Alive\n");
        send_coap_post((uint8_t*)"alive", (uint8_t*)"Alive");
        /* wait 3 seconds */
        xtimer_usleep(BEACON_INTERVAL);
    }
    return NULL;
}

void init_beacon_sender(void)
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
