#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nanocoap.h"
#include "net/gcoap.h"

#include "coap_common.h"

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
