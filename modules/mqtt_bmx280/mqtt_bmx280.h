#ifndef MQTT_BMX280_H
#define MQTT_BMX280_H

#include <stdbool.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void get_temperature(char *value);
void get_pressure(char *value);
#ifdef MODULE_BME280
void get_humidity(char *value);
#endif

void init_bmx280_mqtt_sender(void);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_BMX280_H */
