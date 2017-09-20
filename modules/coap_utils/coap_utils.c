#include <inttypes.h>
#include <stdlib.h>

#include "nanocoap.h"
#include "net/gcoap.h"
#include "coap_utils.h"

#include "od.h"
#include "fmt.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static void _resp_handler(unsigned req_state, coap_pkt_t* pdu,
                          sock_udp_ep_t *remote);

/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu,
                          sock_udp_ep_t *remote)
{
    /* TODO  Nothing? (Copy paste from gcoap example as they are data )*/

#if ENABLE_DEBUG == 1 || 1
    (void)remote;       /* not interested in the source currently */

    if (req_state == GCOAP_MEMO_TIMEOUT) {
        printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        printf("gcoap: error in response\n");
        return;
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    printf("gcoap: response %s, code %1u.%02u", class_str,
                                                coap_get_code_class(pdu),
                                                coap_get_code_detail(pdu));
    if (pdu->payload_len) {
        if (pdu->content_type == COAP_FORMAT_TEXT
                || pdu->content_type == COAP_FORMAT_LINK
                || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
                || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {
            /* Expecting diagnostic payload in failure cases */
            printf(", %u bytes\n%.*s\n", pdu->payload_len, pdu->payload_len,
                                                          (char *)pdu->payload);
        }
        else {
            printf(", %u bytes\n", pdu->payload_len);
            od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
        }
    }
    else {
        printf(", empty payload\n");
    }
#else
    (void) req_state;
    (void) pdu;
    (void) remote;
#endif

}

void send_coap_post(uint8_t* uri_path, uint8_t *data)
{
    /* format destination address from string */
    ipv6_addr_t remote_addr;
    if (ipv6_addr_from_str(&remote_addr, BROKER_ADDR) == NULL) {
        DEBUG("[ERROR]: address not valid '%s'\n", BROKER_ADDR);
        return;
    }

    sock_udp_ep_t remote;

    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;
    remote.port   = BROKER_PORT;

    memcpy(&remote.addr.ipv6[0], &remote_addr.u8[0], sizeof(remote_addr.u8));

    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;
    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, (char*)uri_path);
    memcpy(pdu.payload, (char*)data, strlen((char*)data));
    len = gcoap_finish(&pdu, strlen((char*)data) , COAP_FORMAT_TEXT);

    DEBUG("[INFO] Sending '%s' to '%s:%i%s'\n", data, BROKER_ADDR, BROKER_PORT, uri_path);

    gcoap_req_send2(&buf[0], len, &remote, _resp_handler);

}
