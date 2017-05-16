#include <inttypes.h>
#include <stdlib.h>

#include "nanocoap.h"
#include "net/gcoap.h"
#include "coap_utils.h"

void send_coap_post(uint8_t* uri_path, uint8_t *data)
{
    /* format destination address from string */
    ipv6_addr_t remote_addr;
    sock_udp_ep_t remote;
    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;

    if (ipv6_addr_from_str(&remote_addr, BROKER_ADDR) == NULL) {
        printf("Error: address not valid '%s'\n", BROKER_ADDR);
        return;
    }
    memcpy(&remote.addr.ipv6[0], &remote_addr.u8[0], sizeof(remote_addr.u8));
    remote.port = BROKER_PORT;

    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, (char*)uri_path);
    memcpy(pdu.payload, data, strlen((char*)data));
    len = gcoap_finish(&pdu, strlen((char*)data), COAP_FORMAT_TEXT);
    gcoap_req_send2(buf, len, &remote, NULL);
}
