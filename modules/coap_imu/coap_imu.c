#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nanocoap.h"
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

static phydat_t data[3];
static const char *types[] = {"acc", "mag", "gyro"};

static uint8_t payload[128] = { 0 };
static uint8_t response[128] = { 0 };

void read_imu_values(uint8_t* payload)
{
    /* get sensors */
    saul_reg_t *acc = saul_reg_find_type(SAUL_SENSE_ACCEL);
    saul_reg_t *mag = saul_reg_find_type(SAUL_SENSE_MAG);
    saul_reg_t *gyr = saul_reg_find_type(SAUL_SENSE_GYRO);
    if ((acc == NULL) || (mag == NULL) || (gyr == NULL)) {
        DEBUG("[ERROR] Unable to find sensors\n");
        return;
    }

    saul_reg_read(acc, &data[0]);
    saul_reg_read(mag, &data[1]);
    saul_reg_read(gyr, &data[2]);

    size_t p = 0;
    p += sprintf((char*)&payload[p], "[");
    for (unsigned i = 0; i < 3; ++i) {
        if (i == 1) {
            /* Skip magnetometer */
            continue;
        }
        p += sprintf((char*)&payload[p],
                     "{\"type\":\"%s\",\"values\":[%i, %i, %i]},",
                     types[i], (int)data[i].val[0], (int)data[i].val[1], (int)data[i].val[2]);
    }
    p--;
    p += sprintf((char*)&payload[p], "]");
    payload[p] = '\0';
    return;
}

ssize_t coap_imu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    read_imu_values(pdu->payload);
    size_t payload_len = strlen((char*)pdu->payload);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void *imu_thread(void *args)
{
    msg_init_queue(_imu_msg_queue, IMU_QUEUE_SIZE);

    for(;;) {
        size_t p = 0;
        read_imu_values(payload);
        p += sprintf((char*)&response[p], "imu:");
        p += sprintf((char*)&response[p], (char*)payload);
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
