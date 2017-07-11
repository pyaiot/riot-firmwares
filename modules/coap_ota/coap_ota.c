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

#include "coap_utils.h"
#include "coap_ota.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

ssize_t version_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(FIRMWARE_VERSION);
    memcpy(pdu->payload, FIRMWARE_VERSION, payload_len);

    DEBUG("[DEBUG] Returning firmware version: '%s'", FIRMWARE_VERSION);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t application_id_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(FIRMWARE_APPID);
    memcpy(pdu->payload, FIRMWARE_APPID, payload_len);

    DEBUG("[DEBUG] Returning firmware application ID: '%s'", FIRMWARE_APPID);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}
