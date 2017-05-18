#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nanocoap.h"
#include "net/gcoap.h"

#include "coap_position.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifndef NODE_POSITION
#define NODE_POSITION "{\"lat\":48.714784,\"lng\":2.205502}"
#endif

ssize_t position_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *board = NODE_POSITION;
    size_t payload_len = strlen(NODE_POSITION);
    memcpy(pdu->payload, board, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}
