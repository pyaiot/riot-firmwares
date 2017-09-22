#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "mqtt_bmx280.h"
#include "mqtt_utils.h"

#include "bmx280.h"
#include "bmx280_params.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef APPLICATION_NAME
#define APPLICATION_NAME "Node"
#endif

#define PUBLISH_INTERVAL       (5U)    /* set interval to 5 seconds */

#define PUBLISH_QUEUE_SIZE     (8U)
static msg_t _publish_msg_queue[PUBLISH_QUEUE_SIZE];
static char publish_stack[THREAD_STACKSIZE_DEFAULT];

static bmx280_t bmx280_dev;

void get_temperature(char *value) {
    ssize_t p = 0;
    int16_t temp = bmx280_read_temperature(&bmx280_dev);
    p += sprintf(value, "{\"value\":\"%d.%d°C\"}",
                 temp / 100, (temp % 100) /10);
    value[p] = '\0';
    DEBUG("[DEBUG] Get temperature '%s'\n", value);
}

void get_pressure(char *value) {
    ssize_t p = 0;
    uint32_t pres = bmx280_read_pressure(&bmx280_dev);
    p += sprintf(value, "{\"value\":\"%lu.%dhPa\"}",
                 (unsigned long)pres / 100,
                 (int)pres % 100);
    value[p] = '\0';
    DEBUG("[DEBUG] Get pressure '%s'\n", value);
}

#ifdef MODULE_BME280
void get_humidity(char *value) {
    ssize_t p = 0;
    uint16_t hum = bme280_read_humidity(&bmx280_dev);
    p += sprintf(value, "{\"value\":\"%u.%02u%%\"}",
                 (unsigned int)(hum / 100),
                 (unsigned int)(hum % 100));
    value[p] = '\0';
    DEBUG("[DEBUG] Get humidity '%s'\n", value);
}
#endif

void *publish_thread(void *args)
{
    char topic[64] = { 0 };
    char payload[64] = { 0 };

    msg_init_queue(_publish_msg_queue, PUBLISH_QUEUE_SIZE);
    for(;;) {
        memset(topic, 0, sizeof(topic));
        sprintf(topic, "node/%s/temperature", NODE_ID);
        get_temperature(payload);
        publish((uint8_t*)topic, (uint8_t*)payload);

        xtimer_sleep(1);
        memset(topic, 0, sizeof(topic));
        sprintf(topic, "node/%s/pressure", NODE_ID);
        get_pressure(payload);
        publish((uint8_t*)topic, (uint8_t*)payload);

#ifdef MODULE_BME280
        xtimer_sleep(1);
        memset(topic, 0, sizeof(topic));
        sprintf(topic, "node/%s/humidity", NODE_ID);
        get_humidity(payload);
        publish((uint8_t*)topic, (uint8_t*)payload);
#endif
        /* wait 5 seconds */
        xtimer_sleep(PUBLISH_INTERVAL);
    }
    return NULL;
}

void init_bmx280_mqtt_sender(void)
{
    /* Initialize the BMX280 sensor */
    DEBUG("+------------Initializing BMX280 sensor ------------+\n");
    int result = bmx280_init(&bmx280_dev, &bmx280_params[0]);
    if (result == -1) {
        DEBUG("[ERROR] The given i2c is not enabled\n");
    }
    else if (result == -2) {
        DEBUG("[ERROR] The sensor did not answer correctly on the given address\n");
    }
    else {
        DEBUG("[INFO] Initialization successful\n\n");
    }

    /* create the publish thread that will send periodic measures to
       the broker */
    int publish_pid = thread_create(publish_stack, sizeof(publish_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, publish_thread,
                                   NULL, "Publish thread");
    if (publish_pid == -EINVAL || publish_pid == -EOVERFLOW) {
        DEBUG("[ERROR] Failed to create bmx280 sender thread, exiting\n");
    }
    else {
        DEBUG("[ERROR] Successfuly created beaconing thread !\n");
    }
}