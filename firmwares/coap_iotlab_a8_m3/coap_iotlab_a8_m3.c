#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "nanocoap.h"
#include "net/gcoap.h"

#include "periph/gpio.h"
#include "lsm303dlhc.h"

#include "coap_utils.h"
#include "coap_iotlab_a8_m3.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define I2C_INTERFACE              I2C_DEV(0)    /* I2C interface number */
#define TEMPERATURE_INTERVAL       (5000000U)     /* set temperature updates interval to 5 seconds */

#define IOTLAB_A8_M3_QUEUE_SIZE    (8)

static msg_t _iotlab_a8_m3_msg_queue[IOTLAB_A8_M3_QUEUE_SIZE];
static char iotlab_a8_m3_stack[THREAD_STACKSIZE_DEFAULT];

static lsm303dlhc_t lsm303dlhc_dev;
static uint8_t response[64] = { 0 };


ssize_t lsm303dlhc_temperature_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int16_t temperature = 0;
    lsm303dlhc_read_temp(&lsm303dlhc_dev, &temperature);
    sprintf((char*)response, "%i°C", temperature);
    
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *iotlab_a8_m3_thread(void *args)
{
    msg_init_queue(_iotlab_a8_m3_msg_queue, IOTLAB_A8_M3_QUEUE_SIZE);
    int16_t temperature;

    for(;;) {
        lsm303dlhc_read_temp(&lsm303dlhc_dev, &temperature);
        size_t p = 0;
        p += sprintf((char*)&response[p], "temperature:");
        p += sprintf((char*)&response[p],
                     "%.1f°C", (double)temperature/128.0);
        response[p] = '\0';
        send_coap_post((uint8_t*)"/server", response);;

        /* wait 5 seconds */
        xtimer_usleep(TEMPERATURE_INTERVAL);
    }

    return NULL;
}

void init_iotlab_a8_m3_sender(void)
{
    printf("+------------Initializing temperature device ------------+\n");
    /* Initialise the I2C serial interface as master */
    int init = lsm303dlhc_init(&lsm303dlhc_dev, I2C_INTERFACE,
                               GPIO_PIN(PORT_B,1),
                               GPIO_PIN(PORT_B,2),
                               25,
                               LSM303DLHC_ACC_SAMPLE_RATE_10HZ,
                               LSM303DLHC_ACC_SCALE_2G,
                               30,
                               LSM303DLHC_MAG_SAMPLE_RATE_75HZ,
                               LSM303DLHC_MAG_GAIN_400_355_GAUSS);
    if (init == -1) {
        puts("Error: Init: Given device not available\n");
    }
    else {
        printf("Sensor successfuly initialized!");
    }

    /* create the sensors thread that will send periodic updates to
       the server */
    int iotlab_a8_m3_pid = thread_create(iotlab_a8_m3_stack, sizeof(iotlab_a8_m3_stack),
                                         THREAD_PRIORITY_MAIN - 1,
                                         THREAD_CREATE_STACKTEST, iotlab_a8_m3_thread,
                                         NULL, "iotlab_a8_m3 thread");
    if (iotlab_a8_m3_pid == -EINVAL || iotlab_a8_m3_pid == -EOVERFLOW) {
        puts("Error: failed to create iotlab_a8_m3 thread, exiting\n");
    }
    else {
        puts("Successfuly created iotlab_a8_m3 thread !\n");
    }
}
