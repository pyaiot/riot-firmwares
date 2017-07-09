#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "thread.h"
#include "xtimer.h"

#include "mqtt_common.h"
#include "mqtt_utils.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifndef APPLICATION_NAME
#define APPLICATION_NAME "Node"
#endif

const char *app_name = APPLICATION_NAME;

const char *node_id = NODE_ID;

void get_board(char *value) {
    DEBUG("[DEBUG] return board '%s'", RIOT_BOARD);
    sprintf(value, "%s", RIOT_BOARD);
}

void get_mcu(char *value) {
    DEBUG("[DEBUG] return mcu '%s'", RIOT_MCU);
    sprintf(value, "%s", RIOT_MCU);
}

void get_os(char *value) {
    DEBUG("[DEBUG] return os 'riot'");
    sprintf(value, "riot");
}

void get_name(char *value) {
    DEBUG("[DEBUG] return application name '%s'", app_name);
    sprintf(value, app_name);
}