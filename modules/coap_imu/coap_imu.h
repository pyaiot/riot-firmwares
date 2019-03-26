#ifndef COAP_IMU_H
#define COAP_IMU_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

void read_imu_values(void);

ssize_t coap_imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

void init_imu_sender(void);

#ifdef __cplusplus
}
#endif

#endif
