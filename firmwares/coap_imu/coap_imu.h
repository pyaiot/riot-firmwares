#ifndef COAP_IMU_H
#define COAP_IMU_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void read_imu_values(uint8_t* payload);

ssize_t coap_imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
