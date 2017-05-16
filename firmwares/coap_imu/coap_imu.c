#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nanocoap.h"
#include "net/gcoap.h"

#include "saul_reg.h"

static phydat_t data[3];
static const char *types[] = {"acc", "mag", "gyro"};

void read_imu_values(uint8_t* payload)
{
    /* get sensors */
    saul_reg_t *acc = saul_reg_find_type(SAUL_SENSE_ACCEL);
    saul_reg_t *mag = saul_reg_find_type(SAUL_SENSE_MAG);
    saul_reg_t *gyr = saul_reg_find_type(SAUL_SENSE_GYRO);
    if ((acc == NULL) || (mag == NULL) || (gyr == NULL)) {
        puts("Unable to find sensors");
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
