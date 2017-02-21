/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v3. See the file LICENSE in the top level
 * directory for more details.
 */

#include <coap.h>
#include <string.h>

#include "board.h"
#include "periph_conf.h"
#include "periph/i2c.h"

#define APPLICATION_NAME "I01 XPlained Sensor"

#define MAX_RESPONSE_LEN 500
#define I2C_INTERFACE I2C_DEV(0)    /* I2C interface number */

static bool initialized = 0;

static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

static char temperature[15];

extern int _read_temperature(void);

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi, uint8_t id_lo);

static int handle_get_name(coap_rw_buffer_t *scratch,
                           const coap_packet_t *inpkt,
                           coap_packet_t *outpkt,
                           uint8_t id_hi, uint8_t id_lo);

static int handle_get_os(coap_rw_buffer_t *scratch,
                         const coap_packet_t *inpkt,
                         coap_packet_t *outpkt,
                         uint8_t id_hi, uint8_t id_lo);

static int handle_get_board(coap_rw_buffer_t *scratch,
                            const coap_packet_t *inpkt,
                            coap_packet_t *outpkt,
                            uint8_t id_hi, uint8_t id_lo);

static int handle_get_mcu(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);

static int handle_get_temperature(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_well_known_core =
        { 2, { ".well-known", "core" } };

static const coap_endpoint_path_t path_name =
        { 1, { "name" } };

static const coap_endpoint_path_t path_os =
        { 1, { "os" } };

static const coap_endpoint_path_t path_board =
        { 1, { "board" } };

static const coap_endpoint_path_t path_mcu =
        { 1, { "mcu" } };

static const coap_endpoint_path_t path_temperature =
        { 1, { "temperature" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,	handle_get_well_known_core,
      &path_well_known_core, "ct=40" },
    { COAP_METHOD_GET,	handle_get_name,
      &path_name,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_os,
      &path_os,  	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_board,
      &path_board,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_mcu,
      &path_mcu,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_temperature,
      &path_temperature,   "ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};



void _init_device(void)
{
    if (!initialized ) {
        printf("+------------Initializing device ------------+\n");
        /* Initialise the I2C serial interface as master */
        int init = i2c_init_master(I2C_INTERFACE, I2C_SPEED_NORMAL);
        if (init == -1) {
            puts("Error: Init: Given device not available\n");
        }
        else if (init == -2) {
            puts("Error: Init: Unsupported speed value\n");
        }
        else {
            printf("I2C interface %i successfully initialized as master!\n", I2C_INTERFACE);
            initialized = 1;
        }
    }
}

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi, uint8_t id_lo)
{
    char *rsp = (char *)response;
    /* resetting the content of response message */
    memset(response, 0, sizeof(response));
    size_t len = sizeof(response);
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while (NULL != ep->handler) {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp,
                              strlen(rsp), id_hi, id_lo, &inpkt->tok,
                              COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static int handle_get_name(coap_rw_buffer_t *scratch,
                           const coap_packet_t *inpkt,
                           coap_packet_t *outpkt,
                           uint8_t id_hi, uint8_t id_lo)
{
    const char *app_name = APPLICATION_NAME;
    size_t len = strlen(APPLICATION_NAME);

    memcpy(response, app_name, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_os(coap_rw_buffer_t *scratch,
                         const coap_packet_t *inpkt,
                         coap_packet_t *outpkt,
                         uint8_t id_hi, uint8_t id_lo)
{
    const char *os = "riot";
    size_t len = strlen("riot");

    memcpy(response, os, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_board(coap_rw_buffer_t *scratch,
                            const coap_packet_t *inpkt,
                            coap_packet_t *outpkt,
                            uint8_t id_hi, uint8_t id_lo)
{
    const char *riot_name = RIOT_BOARD;
    int len = strlen(RIOT_BOARD);

    memcpy(response, riot_name, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_mcu(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo)
{
    const char *riot_mcu = RIOT_MCU;
    int len = strlen(RIOT_MCU);
    
    memcpy(response, riot_mcu, len);
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_temperature(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo)
{
    _init_device();
    memset(temperature, 0, sizeof(temperature));
    sprintf(temperature, "%i°C", _read_temperature());

    memcpy(response, temperature,  strlen(temperature));

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(temperature),
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}
