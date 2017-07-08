#include <inttypes.h>
#include <stdlib.h>

#include "nanocoap.h"
#include "net/gcoap.h"
#include "coap_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static sock_udp_t coap_sock;

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

    sock_udp_send(&coap_sock, buf, len, &remote);
}
