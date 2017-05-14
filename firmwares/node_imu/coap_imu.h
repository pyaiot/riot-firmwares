#ifndef COAP_IMU_H
#define COAP_IMU_H

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
ssize_t imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

/* CoAP resources */
static const coap_resource_t _resources[] = {
    { "/name", COAP_GET, name_handler },
    { "/board", COAP_GET, board_handler },
    { "/mcu", COAP_GET, mcu_handler },
    { "/os", COAP_GET, os_handler },
    { "/imu", COAP_GET, imu_handler },
};

#endif /* COAP_IMU_H */
