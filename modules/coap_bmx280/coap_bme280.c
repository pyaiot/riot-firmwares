#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "bmx280_params.h"
#include "bmx280.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_bmx280.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SEND_INTERVAL              (5000000UL)    /* set temperature updates interval to 5 seconds */

#define BMX280_QUEUE_SIZE    (8)

static msg_t _bmx280_msg_queue[BMX280_QUEUE_SIZE];
static char bmx280_stack[THREAD_STACKSIZE_DEFAULT];

static bmx280_t bmx280_dev;
static uint8_t response[64] = { 0 };

static bool use_temperature = false;
static bool use_pressure = false;

#ifdef MODULE_BME280
static bool use_humidity = false;
#endif

ssize_t bmx280_temperature_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int16_t temperature = bmx280_read_temperature(&bmx280_dev);
    p += sprintf((char*)response, "%d.%d°C",
                 temperature / 100, (temperature % 100) /10);
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}

ssize_t bmx280_pressure_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint32_t pressure = bmx280_read_pressure(&bmx280_dev);
    p += sprintf((char*)response, "%lu.%dhPa",
                 (unsigned long)pressure / 100,
                 (int)pressure % 100);
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}

#ifdef MODULE_BME280
ssize_t bmx280_humidity_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint16_t humidity = bme280_read_humidity(&bmx280_dev);
    p += sprintf((char*)response, "%u.%02u%%",
            (unsigned int)(humidity / 100),
            (unsigned int)(humidity % 100));
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}
#endif

void *bmx280_thread(void *args)
{
    msg_init_queue(_bmx280_msg_queue, BMX280_QUEUE_SIZE);

    for(;;) {
        if (use_temperature) {
            ssize_t p = 0;
            int16_t temp = bmx280_read_temperature(&bmx280_dev);
            p += sprintf((char*)&response[p], "temperature:");
            p += sprintf((char*)&response[p], "%d.%d°C",
                         temp / 100, (temp % 100) /10);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        if (use_pressure) {
            ssize_t p = 0;
            uint32_t pres = bmx280_read_pressure(&bmx280_dev);
            p += sprintf((char*)&response[p], "pressure:");
            p += sprintf((char*)&response[p], "%lu.%dhPa",
                         (unsigned long)pres / 100,
                         (int)pres % 100);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

#ifdef MODULE_BME280
        if (use_humidity) {
            ssize_t p = 0;
            uint16_t hum = bme280_read_humidity(&bmx280_dev);
            p += sprintf((char*)&response[p], "humidity:");
            p += sprintf((char*)&response[p], "%u.%02u%%",
                         (unsigned int)(hum / 100),
                         (unsigned int)(hum % 100));
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }
#endif

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_bmx280_sender(bool temperature, bool pressure, bool humidity)
{
    use_temperature = temperature;
    use_pressure = pressure;
#ifdef MODULE_BME280
    use_humidity = humidity;
#endif

    /* Initialize the BMX280 sensor */
    printf("+------------Initializing BMX280 sensor ------------+\n");
    int result = bmx280_init(&bmx280_dev, &bmx280_params[0]);
    if (result == -1) {
        puts("[Error] The given i2c is not enabled");
    }
    else if (result == -2) {
        puts("[Error] The sensor did not answer correctly on the given address");
    }
    else {
        printf("Initialization successful\n\n");
    }

    /* create the sensors thread that will send periodic updates to
       the server */
    int bmx280_pid = thread_create(bmx280_stack, sizeof(bmx280_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, bmx280_thread,
                                   NULL, "bmx280 thread");
    if (bmx280_pid == -EINVAL || bmx280_pid == -EOVERFLOW) {
        puts("Error: failed to create bmx280 thread, exiting\n");
    }
    else {
        puts("Successfuly created bmx280 thread !\n");
    }
}
