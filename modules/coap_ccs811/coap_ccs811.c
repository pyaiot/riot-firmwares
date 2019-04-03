#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"
#include "periph/i2c.h"

#include "ccs811_params.h"
#include "ccs811.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_ccs811.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SEND_INTERVAL        (5000000UL)    /* set updates interval to 5 seconds */

#define CCS811_QUEUE_SIZE    (8)

#define I2C_DEVICE           (0)

static msg_t _ccs811_msg_queue[CCS811_QUEUE_SIZE];
static char ccs811_stack[THREAD_STACKSIZE_DEFAULT];

static ccs811_t ccs811_dev;
static uint8_t response[64] = { 0 };

static bool use_eco2 = false;
static bool use_tvoc = false;

ssize_t ccs811_eco2_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint16_t eco2;
    ccs811_read_iaq(&ccs811_dev, NULL, &eco2, NULL, NULL);
    sprintf((char*)response, "%ippm", eco2);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t ccs811_tvoc_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint16_t tvoc;
    ccs811_read_iaq(&ccs811_dev, &tvoc, NULL, NULL, NULL);
    sprintf((char*)response, "%ippb", tvoc);
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *ccs811_thread(void *args)
{
    (void) args;
    msg_init_queue(_ccs811_msg_queue, CCS811_QUEUE_SIZE);

    for(;;) {
        if (use_eco2) {
            ssize_t p = 0;
            uint16_t eco2;
            ccs811_read_iaq(&ccs811_dev, NULL, &eco2, NULL, NULL);
            p += sprintf((char*)&response[p], "eco2:");
            p += sprintf((char*)&response[p], "%ippm", eco2);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        if (use_tvoc) {
            ssize_t p = 0;
            uint16_t tvoc;
            ccs811_read_iaq(&ccs811_dev, &tvoc, NULL, NULL, NULL);
            p += sprintf((char*)&response[p], "tvoc:");
            p += sprintf((char*)&response[p], "%ippb", tvoc);
            response[p] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }

        /* wait 5 seconds */
        xtimer_usleep(SEND_INTERVAL);
    }

    return NULL;
}

void init_ccs811_sender(bool eco2, bool tvoc)
{
    use_eco2 = eco2;
    use_tvoc = tvoc;

    /* Initialize the CCS811 sensor */
    printf("+------------Initializing CCS811 sensor ------------+\n");
    int result = ccs811_init(&ccs811_dev, &ccs811_params[0]);
    if (result != 0) {
        puts("[Error] Cannot initialize CCS811 sensor");
    }
    else {
        printf("Initialization successful\n\n");
    }

    /* create the sensors thread that will send periodic updates to
       the server */
    int ccs811_pid = thread_create(ccs811_stack, sizeof(ccs811_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, ccs811_thread,
                                   NULL, "ccs811 thread");
    if (ccs811_pid == -EINVAL || ccs811_pid == -EOVERFLOW) {
        puts("Error: failed to create ccs811 thread, exiting\n");
    }
    else {
        puts("Successfuly created ccs811 thread !\n");
    }
}
