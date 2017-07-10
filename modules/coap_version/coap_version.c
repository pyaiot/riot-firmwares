#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"

#include "nanocoap.h"
#include "net/gcoap.h"

#include "ota_update.h"

#include "coap_utils.h"
#include "coap_version.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SEND_INTERVAL        (5000000UL)    /* set updates interval to 5 seconds */

#define VERSION_QUEUE_SIZE    (8)

static msg_t _version_msg_queue[VERSION_QUEUE_SIZE];
static char version_stack[THREAD_STACKSIZE_DEFAULT];

static const char * version = FIRMWARE_VERSION;
static const char * app_id = FIRMWARE_APPID;

static uint8_t payload[16] = { 0 };

ssize_t version_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = sizeof(version);
    memcpy(pdu->payload, version, payload_len);

    DEBUG("[DEBUG] Returning firmware version: '%s'", FIRMWARE_VERSION);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t application_id_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = sizeof(app_id);
    memcpy(pdu->payload, app_id, payload_len);

    DEBUG("[DEBUG] Returning firmware application ID: '%s'", FIRMWARE_APPID);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}


void *version_thread(void *args)
{
    msg_init_queue(_version_msg_queue, VERSION_QUEUE_SIZE);

    for(;;) {
        ssize_t p = 0;
        p += sprintf((char*)&payload[p], "version:%s", version);
        payload[p] = '\0';
        DEBUG("[DEBUG] Sending firmware version: '%s'", FIRMWARE_VERSION);
        send_coap_post((uint8_t*)"/server", payload);

        p = 0;
        p += sprintf((char*)&payload[p], "appid:%s", app_id);
        payload[p] = '\0';
        DEBUG("[DEBUG] Sending firmware application ID: '%s'", FIRMWARE_APPID);
        send_coap_post((uint8_t*)"/server", payload);

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_version_sender(void)
{
    /* create the sensors thread that will send periodic updates to
       the server */
    int version_pid = thread_create(version_stack, sizeof(version_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    THREAD_CREATE_STACKTEST, version_thread,
                                    NULL, "version thread");
    if (version_pid == -EINVAL || version_pid == -EOVERFLOW) {
        puts("Error: failed to create version thread, exiting\n");
    }
    else {
        puts("Successfuly created version thread !\n");
    }
}
