#ifndef COAP_COMMON_H
#define COAP_COMMON_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

void init_beacon_sender(void);

#ifdef __cplusplus
}
#endif

#endif
