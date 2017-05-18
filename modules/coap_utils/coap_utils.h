#ifndef COAP_UTILS_H
#define COAP_UTILS_H

#include <inttypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void send_coap_post(uint8_t* uri_path, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
