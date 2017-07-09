#ifndef MQTT_BME280_H
#define MQTT_BME280_H

#include <stdbool.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void get_temperature(char *value);
void get_pressure(char *value);
void get_humidity(char *value);

void init_bme280_mqtt_sender(void);

#ifdef __cplusplus
}
#endif

#endif
