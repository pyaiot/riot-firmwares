#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int publish(uint8_t *topic, uint8_t *payload);

#ifdef __cplusplus
}
#endif

#endif
