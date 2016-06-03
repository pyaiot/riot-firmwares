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
#include "bmp180.h"

#define MAX_RESPONSE_LEN 500
#define I2C_DEVICE (0)
static bmp180_t bmp180_dev;
static bool initialized = 0;

static uint8_t response[MAX_RESPONSE_LEN] = { 0 };

static char temperature[15];
static char pressure[15];

static int handle_get_well_known_core(coap_rw_buffer_t *scratch,
                                      const coap_packet_t *inpkt,
                                      coap_packet_t *outpkt,
                                      uint8_t id_hi, uint8_t id_lo);

static int handle_get_temperature(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo);

static int handle_get_pressure(coap_rw_buffer_t *scratch,
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

static const coap_endpoint_path_t path_pressure =
        { 1, { "pressure" } };

static const coap_endpoint_path_t path_led =
        { 1, { "led" } };

const coap_endpoint_t endpoints[] =
{
    { COAP_METHOD_GET,	handle_get_well_known_core,
      &path_well_known_core, "ct=40" },
    { COAP_METHOD_GET,	handle_get_temperature,
      &path_temperature,   "ct=0"  },
    { COAP_METHOD_GET,	handle_get_pressure,
      &path_pressure,   "ct=0"  },
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
        uint8_t result = bmp180_init(&bmp180_dev, I2C_DEVICE, BMP180_ULTRALOWPOWER);
        if (result == -1) {
            puts("[Error] The given i2c is not enabled");
            return;
        }
        else if (result == -2) {
            puts("[Error] The sensor did not answer correctly on the given address");
            return;
        }
        else {
            printf("Initialization successful\n\n");
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
    int len = sizeof(response);
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

static int handle_get_temperature(coap_rw_buffer_t *scratch,
                                  const coap_packet_t *inpkt,
                                  coap_packet_t *outpkt,
                                  uint8_t id_hi, uint8_t id_lo)
{
    _init_device();
    int32_t temp;
    memset(temperature, 0, sizeof(temperature));
    bmp180_read_temperature(&bmp180_dev, &temp);
    sprintf(temperature, "%.1f°C", (double)temp/10.0);

    memcpy(response, temperature,  strlen(temperature));

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(temperature),
                              id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT,
                              COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_pressure(coap_rw_buffer_t *scratch,
                               const coap_packet_t *inpkt,
                               coap_packet_t *outpkt,
                               uint8_t id_hi, uint8_t id_lo)
{
    _init_device();
    int32_t pres;
    memset(pressure, 0, sizeof(pressure));
    bmp180_read_pressure(&bmp180_dev, &pres);
    sprintf(pressure, "%.2fhPa", (double)pres/100.0);

    memcpy(response, pressure, strlen(pressure));

    return coap_make_response(scratch, outpkt, (const uint8_t *)response, strlen(pressure),
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
