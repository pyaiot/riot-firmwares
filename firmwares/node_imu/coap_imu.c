/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "net/gcoap.h"
#include "coap_imu.h"

#define APPLICATION_NAME "IMU Unit"

extern void _read_imu(uint8_t* payload);
extern void _send_coap_post(uint8_t* uri_path, uint8_t *data);

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    const char *app_name = APPLICATION_NAME;
    size_t payload_len = strlen(APPLICATION_NAME);
    memcpy(pdu->payload, app_name, payload_len);

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

ssize_t imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    _read_imu(pdu->payload);
    size_t payload_len = strlen((char*)pdu->payload);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}
