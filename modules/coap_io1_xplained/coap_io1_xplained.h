#ifndef COAP_IO1_XPLAINED_H
#define COAP_IO1_XPLAINED_H

#include <inttypes.h>

#include "nanocoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t io1_xplained_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

void read_io1_xplained_temperature(int16_t* temperature);

void init_io1_xplained_temperature_sender(void);

#ifdef __cplusplus
}
#endif

#endif /* COAP_IO1_XPLAINED_H */
