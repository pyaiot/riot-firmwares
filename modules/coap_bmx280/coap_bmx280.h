#ifndef COAP_BMX280_H
#define COAP_BMX280_H

#include <stdbool.h>
#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t bmx280_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
ssize_t bmx280_pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
#ifdef MODULE_BME280
ssize_t bmx280_humidity_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
#endif

void init_bmx280_sender(bool temperature, bool pressure, bool humidity);
void bmx280_handler(void *args);

#ifdef __cplusplus
}
#endif

#endif /* COAP_BMX280_H */
