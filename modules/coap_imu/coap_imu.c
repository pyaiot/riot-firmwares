#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"

#include "saul_reg.h"

#include "debug.h"
#define ENABLE_DEBUG   (0)

#include "coap_common.h"
#include "coap_utils.h"

#define IMU_INTERVAL          (200000U)      /* set imu refresh interval to 200 ms */
#define IMU_QUEUE_SIZE        (8U)

static msg_t _imu_msg_queue[IMU_QUEUE_SIZE];
static char imu_stack[THREAD_STACKSIZE_DEFAULT];

static phydat_t data[2];
static uint8_t response[128];

void read_imu_values(void)
{
    /* get sensors */
    saul_reg_t *acc = saul_reg_find_type(SAUL_SENSE_ACCEL);
    saul_reg_t *gyr = saul_reg_find_type(SAUL_SENSE_GYRO);
    if ((acc == NULL) || (gyr == NULL)) {
        DEBUG("[ERROR] Unable to find sensors\n");
        return;
    }

    saul_reg_read(acc, &data[0]);
    saul_reg_read(gyr, &data[1]);
    return;
}

ssize_t coap_imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    read_imu_values();
    memset(response, 0, sizeof(response));
    size_t p = 0;
    p += sprintf((char*)&response[p], "imu:");
    p += sprintf((char*)&response[p],
                 "[{\"type\":\"acc\",\"values\":[%i,%i,%i]}]",
                 data[0].val[0], data[0].val[1], data[0].val[2]);
    response[p] = '\0';
    size_t payload_len = strlen((char*)response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *imu_thread(void *args)
{
        (void)args;

    msg_init_queue(_imu_msg_queue, IMU_QUEUE_SIZE);

    for(;;) {
        size_t p = 0;
        memset(response, strlen((char*)response), 0);
        read_imu_values();
        p += sprintf((char*)&response[p], "imu:");
        p += sprintf((char*)&response[p],
                 "[{\"type\":\"acc\",\"values\":[%i,%i,%i]}]",
                 data[0].val[0], data[0].val[1], data[0].val[2]);
        response[p] = '\0';
        send_coap_post((uint8_t*)"/server", response);

        p = 0;
        memset(response, strlen((char*)response), 0);
        p += sprintf((char*)&response[p], "imu:");
        p += sprintf((char*)&response[p],
                 "[{\"type\":\"gyro\",\"values\":[%i,%i,%i]}]",
                 data[1].val[0], data[1].val[1], data[1].val[2]);
        response[p] = '\0';
        send_coap_post((uint8_t*)"/server", response);
        /* wait 3 seconds */
        xtimer_usleep(IMU_INTERVAL);
    }
    return NULL;
}

void init_imu_sender(void)
{
    /* create the sensors thread that will send periodic updates to
       the server */
    int imu_pid = thread_create(imu_stack, sizeof(imu_stack),
                                THREAD_PRIORITY_MAIN - 1,
                                THREAD_CREATE_STACKTEST, imu_thread,
                                NULL, "IMU thread");
    if (imu_pid == -EINVAL || imu_pid == -EOVERFLOW) {
        puts("Error: failed to create imu thread, exiting\n");
    }
    else {
        puts("Successfuly created imu thread !\n");
    }
}
