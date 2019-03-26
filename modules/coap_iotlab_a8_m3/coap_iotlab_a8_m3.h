#ifndef COAP_IOTLAB_A8_M3_H
#define COAP_IOTLAB_A8_M3_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t lsm303dlhc_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

void init_iotlab_a8_m3_sender(void);

#ifdef __cplusplus
}
#endif

#endif /* COAP_IOTLAB_A8_M3_H */
