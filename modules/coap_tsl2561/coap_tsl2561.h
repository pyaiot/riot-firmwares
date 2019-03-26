#ifndef COAP_TSL2561_H
#define COAP_TSL2561_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t tsl2561_illuminance_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

void init_tsl2561_sender(void);

#ifdef __cplusplus
}
#endif

#endif /* COAP_TSL2561_H */
