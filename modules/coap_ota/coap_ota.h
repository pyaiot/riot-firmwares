#ifndef COAP_OTA_H
#define COAP_OTA_H

#include <stdbool.h>
#include <inttypes.h>

#include "nanocoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t version_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t application_id_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

void init_version_sender(void);

#ifdef __cplusplus
}
#endif

#endif /* COAP_OTA_H */
