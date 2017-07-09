#ifndef MQTT_COMMON_H
#define MQTT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

void get_board(char *value);
void get_mcu(char *value);
void get_os(char *value);
void get_name(char *value);

void init_beacon_sender(void);

#ifdef __cplusplus
}
#endif

#endif
