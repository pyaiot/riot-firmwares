#ifndef COAP_CCS811_H
#define COAP_CCS811_H

#include <stdbool.h>
#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t ccs811_eco2_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
ssize_t ccs811_tvoc_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

void init_ccs811_sender(bool eco2, bool tvoc);
void ccs811_handler(void *args);

#ifdef __cplusplus
}
#endif

#endif /* COAP_CCS811_H */
