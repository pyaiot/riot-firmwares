#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "periph/gpio.h"

#include "fmt.h"

#include "nanocoap.h"
#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_utils.h"
#include "coap_led.h"

ssize_t coap_led_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
    unsigned code = COAP_CODE_EMPTY;
    size_t payload_len = pdu->payload_len;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    switch(method_flag) {
    case COAP_GET:
        /* write the response buffer with the internal value */
        payload_len = fmt_u16_dec((char *)pdu->payload, gpio_read(LED0_PIN) == 0);
        code = COAP_CODE_205;
        break;
    case COAP_POST:
    {
        /* convert the payload to an integer and update the internal value */
        char payload[16] = { 0 };
        memcpy(payload, (char*)pdu->payload, pdu->payload_len);
        uint8_t val = strtol(payload, NULL, 10);
        if ( (pdu->payload_len == 1) &&
             ((val == 1) || (val == 0))) {
            /* update LED value */
            gpio_write(LED0_PIN, 1 - val);
        }
        else {
            code = COAP_CODE_BAD_REQUEST;
        }
        code = COAP_CODE_CHANGED;
        break;
    }
    default:
        code = COAP_CODE_BAD_REQUEST;
        break;
    }

    gcoap_resp_init(pdu, buf, len, code);
    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}
