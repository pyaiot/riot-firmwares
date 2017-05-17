#include <inttypes.h>
#include <stdlib.h>

#include "nanocoap.h"
#include "net/gcoap.h"
#include "coap_utils.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static void _resp_handler(unsigned req_state, coap_pkt_t* pdu)
{
    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS) ? "Success" : "Error";
    DEBUG("[INFO] Received '%s', code %1u.%02u\n",
          class_str, coap_get_code_class(pdu), coap_get_code_detail(pdu));
    return;
}

void send_coap_post(uint8_t* uri_path, uint8_t *data)
{
    /* format destination address from string */
    ipv6_addr_t remote_addr;
    if (ipv6_addr_from_str(&remote_addr, BROKER_ADDR) == NULL) {
        DEBUG("[ERROR]: address not valid '%s'\n", BROKER_ADDR);
        return;
    }

    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;
    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, (char*)uri_path);
    memcpy(pdu.payload, (char*)data, strlen((char*)data));
    len = gcoap_finish(&pdu, strlen((char*)data) , COAP_FORMAT_TEXT);

    DEBUG("[INFO] Sending '%s' to '%s:%i%s'\n", data, BROKER_ADDR, BROKER_PORT, uri_path);
    gcoap_req_send(buf, len, &remote_addr, BROKER_PORT, _resp_handler);
}
