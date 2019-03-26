#ifndef COAP_LED_H
#define COAP_LED_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t led_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
