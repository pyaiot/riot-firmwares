#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "periph/gpio.h"

#include "fmt.h"

#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_led.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

ssize_t led_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    ssize_t p = 0;
    char rsp[16];
    unsigned code = COAP_CODE_EMPTY;

    /* read coap method type in packet */
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    switch(method_flag) {
    case COAP_GET:
    {
        p += sprintf(rsp, "%i", gpio_read(LED0_PIN) == 0);
        DEBUG("[DEBUG] Returning LED value '%s'\n", rsp);
        code = COAP_CODE_205;
        break;
    }
    case COAP_PUT:
    case COAP_POST:
    {
        /* convert the payload to an integer and update the internal value */
        char payload[16] = { 0 };
        memcpy(payload, (char*)pdu->payload, pdu->payload_len);
        uint8_t val = strtol(payload, NULL, 10);
        if ( (pdu->payload_len == 1) &&
             ((val == 1) || (val == 0))) {
            /* update LED value */
            DEBUG("[DEBUG] Update LED value '%i'\n", 1 - val);
            gpio_write(LED0_PIN, 1 - val);
            code = COAP_CODE_CHANGED;
            p += sprintf(rsp, "led:%i", val);
        }
        else {
            DEBUG("[ERROR] Wrong LED value given '%i'\n", val);
            code = COAP_CODE_BAD_REQUEST;
        }
        break;
    }
    default:
        DEBUG("[Error] Bad request\n");
        code = COAP_CODE_BAD_REQUEST;
        break;
    }

    return coap_reply_simple(pdu, code, buf, len, COAP_FORMAT_TEXT, (uint8_t*)rsp, p);
}
