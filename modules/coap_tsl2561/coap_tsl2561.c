#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "tsl2561_params.h"
#include "tsl2561.h"

#include "net/gcoap.h"

#include "board.h"

#include "coap_utils.h"
#include "coap_tsl2561.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SEND_INTERVAL        (5000000UL)    /* set updates interval to 5 seconds */

#define TSL2561_QUEUE_SIZE    (8)

/* TSL2561 sensor */
#define I2C_DEVICE (0)

static msg_t _tsl2561_msg_queue[TSL2561_QUEUE_SIZE];
static char tsl2561_stack[THREAD_STACKSIZE_DEFAULT];

static tsl2561_t tsl2561_dev;
static uint8_t response[64] = { 0 };

ssize_t tsl2561_illuminance_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    sprintf((char*)response, "%ilx", (int)tsl2561_read_illuminance(&tsl2561_dev));
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *tsl2561_thread(void *args)
{
    (void)args;
    msg_init_queue(_tsl2561_msg_queue, TSL2561_QUEUE_SIZE);

    for(;;) {
        size_t p = 0;
        p += sprintf((char*)&response[p], "illuminance:");
        p += sprintf((char*)&response[p],
                     "%ilx", (int)tsl2561_read_illuminance(&tsl2561_dev));
        response[p] = '\0';
        send_coap_post((uint8_t*)"/server", response);

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_tsl2561_sender(void)
{
    /* Initialize the TSL2561 sensor */
    printf("+------------Initializing TSL2561 sensor ------------+\n");
    int result = tsl2561_init(&tsl2561_dev, &tsl2561_params[0]);
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
    int tsl2561_pid = thread_create(tsl2561_stack, sizeof(tsl2561_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    THREAD_CREATE_STACKTEST, tsl2561_thread,
                                    NULL, "tsl2561 thread");
    if (tsl2561_pid == -EINVAL || tsl2561_pid == -EOVERFLOW) {
        puts("Error: failed to create tsl2561 thread, exiting\n");
    }
    else {
        puts("Successfuly created tsl2561 thread !\n");
    }
}
