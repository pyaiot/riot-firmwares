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
#include "periph_conf.h"
#include "lsm303dlhc.h"

#define MAX_RESPONSE_LEN 500
#define I2C_INTERFACE I2C_DEV(0)    /* I2C interface number */

static lsm303dlhc_t lsm303dlhc_dev;
static bool initialized = 0;

static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

static char temperature[15];

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi, uint8_t id_lo);

static int handle_get_temperature(coap_rw_buffer_t *scratch,
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

static const coap_endpoint_path_t path_temperature =
        { 1, { "temperature" } };

static const coap_endpoint_path_t path_led =
        { 1, { "led" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,	handle_get_well_known_core,
      &path_well_known_core, "ct=40" },
    { COAP_METHOD_GET,	handle_get_temperature,
      &path_temperature,   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_led,
      &path_led,	"ct=0"  },
    { COAP_METHOD_PUT,	handle_put_led,
      &path_led,	"ct=0"  },
    /* marks the end of the endpoints array: */
    { (coap_method_t)0, NULL, NULL, NULL }
};



void _init_device(void)
{
    if (!initialized ) {
        printf("+------------Initializing device ------------+\n");
        /* Initialise the I2C serial interface as master */
        int init = lsm303dlhc_init(&lsm303dlhc_dev, I2C_INTERFACE,
                                   GPIO_PIN(PORT_B,1),
                                   GPIO_PIN(PORT_B,2),
                                   25,
                                   LSM303DLHC_ACC_SAMPLE_RATE_10HZ,
                                   LSM303DLHC_ACC_SCALE_2G,
                                   30,
                                   LSM303DLHC_MAG_SAMPLE_RATE_75HZ,
                                   LSM303DLHC_MAG_GAIN_400_355_GAUSS);
        if (init == -1) {
            puts("Error: Init: Given device not available\n");
        }
        else {
            printf("Sensor successfuly initialized!");
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

    strncat(rsp, "],\"board\":\"", 11);
    len -= 11;

    strncat(rsp, RIOT_BOARD, strlen(RIOT_BOARD));
    len -= strlen(RIOT_BOARD);
    
    strncat(rsp, "\"}", 2);
    len -= 2;
    
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp,
                              strlen(rsp), id_hi, id_lo, &inpkt->tok,
                              COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static int handle_get_temperature(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo)
{
    int16_t temp;
    _init_device();
    memset(temperature, 0, sizeof(temperature));
    lsm303dlhc_read_temp(&lsm303dlhc_dev, &temp);
    sprintf(temperature, "%.1f°C", (double)temp/128.0);

    memcpy(response, temperature,  strlen(temperature));

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(temperature),
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
