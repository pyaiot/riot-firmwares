#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"

#include "coap_position.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef NODE_LAT
#define NODE_LAT "48.714784"
#endif
#ifndef NODE_LNG
#define NODE_LNG "2.205502"
#endif

static uint8_t response[64] = { 0 };

ssize_t position_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    p += sprintf((char*)response, "{\"lat\":%s,\"lng\":%s}",
                 NODE_LAT, NODE_LNG);
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}
