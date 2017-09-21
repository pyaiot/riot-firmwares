#ifndef COAP_BME280_H
#define COAP_BME280_H

#include <stdbool.h>
#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t bme280_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t bme280_pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t bme280_humidity_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

void init_bme280_sender(bool temperature, bool pressure, bool humidity);

#ifdef __cplusplus
}
#endif

#endif /* COAP_BME280_H */
