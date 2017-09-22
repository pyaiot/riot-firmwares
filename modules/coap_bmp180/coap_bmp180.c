#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "bmp180_params.h"
#include "bmp180.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_bmp180.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SEND_INTERVAL        (5000000UL)    /* set updates interval to 5 seconds */

#define BMP180_QUEUE_SIZE    (8)

#define I2C_DEVICE           (0)

static msg_t _bmp180_msg_queue[BMP180_QUEUE_SIZE];
static char bmp180_stack[THREAD_STACKSIZE_DEFAULT];

static bmp180_t bmp180_dev;
static uint8_t response[64] = { 0 };

static bool use_temperature = false;
static bool use_pressure = false;

ssize_t bmp180_temperature_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int32_t temperature = bmp180_read_temperature(&bmp180_dev);
    sprintf((char*)response, "%.1f°C", (double)temperature / 10.0);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t bmp180_pressure_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int32_t pressure = bmp180_read_pressure(&bmp180_dev);
    sprintf((char*)response, "%.2fhPa", (double)pressure / 100);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *bmp180_thread(void *args)
{
    msg_init_queue(_bmp180_msg_queue, BMP180_QUEUE_SIZE);

    for(;;) {
        if (use_temperature) {
            ssize_t p = 0;
            int32_t temperature = bmp180_read_temperature(&bmp180_dev);
            p += sprintf((char*)&response[p], "temperature:");
            p += sprintf((char*)&response[p], "%.1f°C", (double)temperature / 10);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        if (use_pressure) {
            ssize_t p = 0;
            int32_t pressure = bmp180_read_pressure(&bmp180_dev);
            p += sprintf((char*)&response[p], "pressure:");
            p += sprintf((char*)&response[p], "%.2fhPa", (double)pressure / 100);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_bmp180_sender(bool temperature, bool pressure)
{
    use_temperature = temperature;
    use_pressure = pressure;

    /* Initialize the BMP180 sensor */
    printf("+------------Initializing BMP180 sensor ------------+\n");
    int result = bmp180_init(&bmp180_dev, &bmp180_params[0]);
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
    int bmp180_pid = thread_create(bmp180_stack, sizeof(bmp180_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, bmp180_thread,
                                   NULL, "bmp180 thread");
    if (bmp180_pid == -EINVAL || bmp180_pid == -EOVERFLOW) {
        puts("Error: failed to create bmp180 thread, exiting\n");
    }
    else {
        puts("Successfuly created bmp180 thread !\n");
    }
}
