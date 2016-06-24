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

#define MAX_RESPONSE_LEN 500
static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi, uint8_t id_lo);

static int handle_get_riot_board(coap_rw_buffer_t *scratch,
                                 const coap_packet_t *inpkt,
                                 coap_packet_t *outpkt,
                                 uint8_t id_hi, uint8_t id_lo);

static int handle_get_riot_mcu(coap_rw_buffer_t *scratch,
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

static const coap_endpoint_path_t path_well_known_core =
        { 2, { ".well-known", "core" } };

static const coap_endpoint_path_t path_riot_board =
        { 2, { "riot", "board" } };

static const coap_endpoint_path_t path_riot_mcu =
        { 2, { "riot", "mcu" } };

static const coap_endpoint_path_t path_led =
        { 1, { "led" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,	handle_get_well_known_core,
      &path_well_known_core, "ct=40" },
    { COAP_METHOD_GET,	handle_get_riot_board,
      &path_riot_board,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_riot_mcu,
      &path_riot_mcu,	   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_led,
      &path_led,	   "ct=0"  },
    { COAP_METHOD_PUT,	handle_put_led,
      &path_led,	   "ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    char *rsp = (char *)response;
    // resetting the content of response message
    memset(response, 0, sizeof(response));
    int len = sizeof(response);
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    strncat(rsp, "{\"paths\":[", 10);
    len -= 10;

    while (NULL != ep->handler) {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (10 < strlen(rsp)) {
            strncat(rsp, ",", 1);
            len--;
        }

        strncat(rsp, "{\"path\":\"", 9);
        len-=9;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", 1);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, "\",", 2);
        len -= 2;

        strncat(rsp, "\"method\":", 9);
        len-=9;
        switch (ep->method) {
        case COAP_METHOD_GET:
            strncat(rsp, "\"GET\"", 5);
            len -= 5;
            break;
        case COAP_METHOD_PUT:
            strncat(rsp, "\"PUT\"", 5);
            len -= 5;
            break;
        default:
            break;
        }
        
        strncat(rsp, "}", 1);
        len--;
        ep++;
    }

    strncat(rsp, "]}", 2);
    len -= 2;

    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp,
                              strlen(rsp), id_hi, id_lo, &inpkt->tok,
                              COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static int handle_get_riot_board(coap_rw_buffer_t *scratch,
        const coap_packet_t *inpkt, coap_packet_t *outpkt,
        uint8_t id_hi, uint8_t id_lo)
{
    const char *riot_name = RIOT_BOARD;
    int len = strlen(RIOT_BOARD);

    memcpy(response, riot_name, len);

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, len,
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_riot_mcu(coap_rw_buffer_t *scratch,
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
