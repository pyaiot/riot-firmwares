#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "bme280_params.h"
#include "bme280.h"

#include "nanocoap.h"
#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_bme280.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define SEND_INTERVAL              (5000000UL)    /* set temperature updates interval to 5 seconds */

#define BME280_QUEUE_SIZE    (8)

static msg_t _bme280_msg_queue[BME280_QUEUE_SIZE];
static char bme280_stack[THREAD_STACKSIZE_DEFAULT];

static bme280_t bme280_dev;
static uint8_t response[64] = { 0 };

static bool use_temperature = false;
static bool use_pressure = false;
static bool use_humidity = false;

ssize_t bme280_temperature_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int16_t temperature = bme280_read_temperature(&bme280_dev);
    sprintf((char*)response, "%d.%d°C",
            temperature / 100, (temperature % 100) /10);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t bme280_pressure_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint32_t pressure = bme280_read_pressure(&bme280_dev);
    sprintf((char*)response, "%lu.%dhPa",
            (unsigned long)pressure / 100,
            (int)pressure % 100);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t bme280_humidity_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint16_t humidity = bme280_read_humidity(&bme280_dev);
    sprintf((char*)response, "%u.%02u%%",
            (unsigned int)(humidity / 100),
            (unsigned int)(humidity % 100));
    
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *bme280_thread(void *args)
{
    msg_init_queue(_bme280_msg_queue, BME280_QUEUE_SIZE);

    for(;;) {
        if (use_temperature) {
            ssize_t p = 0;
            int16_t temp = bme280_read_temperature(&bme280_dev);
            p += sprintf((char*)&response[p], "temperature:");
            p += sprintf((char*)&response[p], "%d.%d°C",
                         temp / 100, (temp % 100) /10);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        if (use_pressure) {
            ssize_t p = 0;
            uint32_t pres = bme280_read_pressure(&bme280_dev);
            p += sprintf((char*)&response[p], "pressure:");
            p += sprintf((char*)&response[p], "%lu.%dhPa",
                         (unsigned long)pres / 100,
                         (int)pres % 100);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        if (use_humidity) {
            ssize_t p = 0;
            uint16_t hum = bme280_read_humidity(&bme280_dev);
            p += sprintf((char*)&response[p], "humidity:");
            p += sprintf((char*)&response[p], "%u.%02u%%",
                         (unsigned int)(hum / 100),
                         (unsigned int)(hum % 100));
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_bme280_sender(bool temperature, bool pressure, bool humidity)
{
    use_temperature = temperature;
    use_pressure = pressure;
    use_humidity = humidity;

    /* Initialize the BME280 sensor */
    printf("+------------Initializing BME280 sensor ------------+\n");
    int result = bme280_init(&bme280_dev, &bme280_params[0]);
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
    int bme280_pid = thread_create(bme280_stack, sizeof(bme280_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, bme280_thread,
                                   NULL, "bme280 thread");
    if (bme280_pid == -EINVAL || bme280_pid == -EOVERFLOW) {
        puts("Error: failed to create bme280 thread, exiting\n");
    }
    else {
        puts("Successfuly created bme280 thread !\n");
    }
}
