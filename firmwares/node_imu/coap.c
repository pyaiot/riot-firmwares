/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <coap.h>
#include <string.h>

#include "board.h"
#include "periph/gpio.h"

#include "saul_reg.h"

#define MAX_RESPONSE_LEN 500
static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

static char payload[512];
static phydat_t data[3];
static const char *types[] = {"acc", "mag", "gyro"};

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
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

static int handle_get_led(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);

static int handle_put_led(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);

static int handle_get_imu(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_well_known_core =
        { 2, { ".well-known", "core" } };

static const coap_endpoint_path_t path_board =
        { 1, { "board" } };

static const coap_endpoint_path_t path_mcu =
        { 1, { "mcu" } };

static const coap_endpoint_path_t path_led =
        { 1, { "led" } };

static const coap_endpoint_path_t path_imu =
        { 1, { "imu" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,	handle_get_well_known_core,
      &path_well_known_core, "ct=40" },
    { COAP_METHOD_GET,	handle_get_board,
      &path_board,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_mcu,
      &path_mcu,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_led,
      &path_led,	   "ct=0"  },
    { COAP_METHOD_PUT,	handle_put_led,
      &path_led,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_imu,
      &path_imu,	   "ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
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

static int handle_get_board(coap_rw_buffer_t *scratch,
                            const coap_packet_t *inpkt,
                            coap_packet_t *outpkt,
                            uint8_t id_hi, uint8_t id_lo)
{
    const char *riot_name = RIOT_BOARD;
    size_t len = strlen(RIOT_BOARD);

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
    size_t len = strlen(RIOT_MCU);
    
    memcpy(response, riot_mcu, len);
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_led(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo)
{
    char led_status[1];
    sprintf(led_status, "%d", 1 - gpio_read(LED0_PIN));
    memcpy(response, led_status, 1);
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)response, 1,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_led(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo)
{
  coap_responsecode_t resp = COAP_RSPCODE_CHANGED;
  
  /* On vérifie que la valeur donnée est correcte (0 ou 1)*/
  uint8_t val = inpkt->payload.p[0];
  if ((inpkt->payload.len == 1) &&
    ((val == '1') || (val == '0'))) {
    /* écriture de la nouvelle valeur de la led */
    gpio_write(LED0_PIN, (val - '1'));
  }
  else {
    resp = COAP_RSPCODE_BAD_REQUEST;
  }

  /* Réponse faite au client */
  return coap_make_response(scratch, outpkt, NULL, 0,
                            id_hi, id_lo,
                            &inpkt->tok, resp,
                            COAP_CONTENTTYPE_TEXT_PLAIN);
}


static int handle_get_imu(coap_rw_buffer_t *scratch,
                          const coap_packet_t *inpkt,
                          coap_packet_t *outpkt,
                          uint8_t id_hi, uint8_t id_lo)
{
    /* get sensors */
    saul_reg_t *acc = saul_reg_find_type(SAUL_SENSE_ACCEL);
    saul_reg_t *mag = saul_reg_find_type(SAUL_SENSE_MAG);
    saul_reg_t *gyr = saul_reg_find_type(SAUL_SENSE_GYRO);
    if ((acc == NULL) || (mag == NULL) || (gyr == NULL)) {
        puts("Unable to find sensors");
        return 1;
    }

    saul_reg_read(acc, &data[0]);
    saul_reg_read(mag, &data[1]);
    saul_reg_read(gyr, &data[2]);

    size_t p = 0;
    p += sprintf(&payload[p], "[");
    for (int i = 0; i < 3; i++) {
        p += sprintf(&payload[p],
                     "{\"type\":\"%s\", \"values\":[%i, %i, %i]},",
                     types[i], (int)data[i].val[0], (int)data[i].val[1], (int)data[i].val[2]);
    }
    p--;
    p += sprintf(&payload[p], "]");
    payload[p] = '\0';

    int len = strlen(payload);
    memcpy(response, payload, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}