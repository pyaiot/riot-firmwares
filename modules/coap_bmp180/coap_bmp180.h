#ifndef COAP_BMP180_H
#define COAP_BMP180_H

#include <stdbool.h>
#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t bmp180_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
ssize_t bmp180_pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

void init_bmp180_sender(bool temperature, bool pressure);

#ifdef __cplusplus
}
#endif

#endif /* COAP_BMP180_H */
