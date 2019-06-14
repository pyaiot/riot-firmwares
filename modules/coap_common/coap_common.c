#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fmt.h"
#include "luid.h"
#include "thread.h"
#include "net/ieee802154.h"
#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef APPLICATION_NAME
#define APPLICATION_NAME "Node"
#endif

#ifndef MODULE_SCHEDREG
#define BEACON_INTERVAL       (30000000U)    /* set interval to 30 seconds */

#define BEACONING_QUEUE_SIZE  (8U)
static msg_t _beaconing_msg_queue[BEACONING_QUEUE_SIZE];
static char beaconing_stack[THREAD_STACKSIZE_DEFAULT];
#endif

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'name' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(APPLICATION_NAME);
    memcpy(pdu->payload, APPLICATION_NAME, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'board' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *board = RIOT_BOARD;
    size_t payload_len = strlen(RIOT_BOARD);
    memcpy(pdu->payload, board, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'mcu' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *mcu = RIOT_MCU;
    size_t payload_len = strlen(RIOT_MCU);
    memcpy(pdu->payload, mcu, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'os' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *os = "riot";
    size_t payload_len = strlen("riot");
    memcpy(pdu->payload, os, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void beacon_handler(void *arg)
{
    (void) arg;
    uint8_t addr[IEEE802154_LONG_ADDRESS_LEN];
    char uid[IEEE802154_LONG_ADDRESS_LEN * 2];
    luid_get(addr, IEEE802154_LONG_ADDRESS_LEN);
    fmt_bytes_hex(uid, addr, IEEE802154_LONG_ADDRESS_LEN);
    size_t msg_len = strlen("alive:") + (IEEE802154_LONG_ADDRESS_LEN * 2);
    char alive_msg[msg_len];
    snprintf(alive_msg, msg_len, "alive:%s", uid);
    /* Schedule next transmission */
    send_coap_post((uint8_t*)"/alive", (uint8_t*)alive_msg);
}

#ifndef MODULE_SCHEDREG
void *beaconing_thread(void *args)
{
    msg_init_queue(_beaconing_msg_queue, BEACONING_QUEUE_SIZE);

    char *uid = (char *)args;
    size_t msg_len = strlen("alive:") + (IEEE802154_LONG_ADDRESS_LEN * 2);
    char alive_msg[msg_len];
    snprintf(alive_msg, msg_len, "alive:%s", uid);

    for(;;) {
        DEBUG("[DEBUG] common: sending beacon\n");
        send_coap_post((uint8_t*)"/alive", (uint8_t*)alive_msg);
        /* wait 30 seconds */
        xtimer_usleep(BEACON_INTERVAL);
    }
    return NULL;
}
#endif

void init_beacon_sender(void)
{
    uint8_t addr[IEEE802154_LONG_ADDRESS_LEN];
    char uid[IEEE802154_LONG_ADDRESS_LEN * 2];
    luid_get(addr, IEEE802154_LONG_ADDRESS_LEN);
    fmt_bytes_hex(uid, addr, IEEE802154_LONG_ADDRESS_LEN);
    size_t msg_len = strlen("reset:") + (IEEE802154_LONG_ADDRESS_LEN * 2);
    char reset_msg[msg_len];
    snprintf(reset_msg, msg_len, "reset:%s", uid);
    send_coap_post((uint8_t*)"/alive", (uint8_t*)reset_msg);

#ifndef MODULE_SCHEDREG
    /* create the beaconning thread that will send periodic messages to
       the broker */
    int beacon_pid = thread_create(beaconing_stack, sizeof(beaconing_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, beaconing_thread,
                                   uid, "Beaconing thread");
    if (beacon_pid == -EINVAL || beacon_pid == -EOVERFLOW) {
        puts("Error: failed to create beaconing thread, exiting\n");
    }
    else {
        puts("Successfully created beaconing thread !\n");
    }
#endif
}
