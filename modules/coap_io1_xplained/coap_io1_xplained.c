#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_io1_xplained.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define I2C_INTERFACE              I2C_DEV(0)    /* I2C interface number */
#define SENSOR_ADDR                (0x48 | 0x07) /* I2C temperature address on sensor */

#define TEMPERATURE_INTERVAL       (5000000U)     /* set temperature updates interval to 5 seconds */

#define IO1_XPLAINED_QUEUE_SIZE    (8)

static msg_t _io1_xplained_msg_queue[IO1_XPLAINED_QUEUE_SIZE];
static char io1_xplained_stack[THREAD_STACKSIZE_DEFAULT];

static char response[64];

ssize_t io1_xplained_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    memset(response, 0, sizeof(response));
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    int16_t temperature;
    read_io1_xplained_temperature(&temperature);
    size_t p = 0;
    p += sprintf(&response[p], "%i°C", temperature);
    response[p] = '\0';

    size_t payload_len = strlen(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void read_io1_xplained_temperature(int16_t *temperature)
{
    char buffer[2] = { 0 };
    /* read temperature register on I2C bus */
    if (i2c_read_bytes(I2C_INTERFACE, SENSOR_ADDR, buffer, 2, 0) < 0) {
        printf("Error: cannot read at address %i on I2C interface %i\n",
               SENSOR_ADDR, I2C_INTERFACE);
        return;
    }
    
    uint16_t data = (buffer[0] << 8) | buffer[1];
    int8_t sign = 1;
    /* Check if negative and clear sign bit. */
    if (data & (1 << 15)) {
        sign *= -1;
        data &= ~(1 << 15);
    }
    /* Convert to temperature */
    data = (data >> 5);
    *temperature = data * sign * 0.125;

    return;
}

void *io1_xplained_thread(void *args)
{
    (void)args;
    msg_init_queue(_io1_xplained_msg_queue, IO1_XPLAINED_QUEUE_SIZE);
    
    for(;;) {
        int16_t temperature;
        read_io1_xplained_temperature(&temperature);
        size_t p = 0;
        p += sprintf((char*)&response[p], "temperature:%i°C",
                     temperature);
        response[p] = '\0';
        send_coap_post((uint8_t*)"/server", (uint8_t*)response);
        /* wait 3 seconds */
        xtimer_usleep(TEMPERATURE_INTERVAL);
    }
    return NULL;
}

void init_io1_xplained_temperature_sender(void)
{
    /* create the sensors thread that will send periodic updates to
       the server */
    int io1_xplained_pid = thread_create(io1_xplained_stack, sizeof(io1_xplained_stack),
                                         THREAD_PRIORITY_MAIN - 1,
                                         THREAD_CREATE_STACKTEST, io1_xplained_thread,
                                         NULL, "io1_xplained thread");
    if (io1_xplained_pid == -EINVAL || io1_xplained_pid == -EOVERFLOW) {
        puts("Error: failed to create io1_xplained thread, exiting");
    }
    else {
        puts("Successfuly created io1_xplained thread !");
    }
}
