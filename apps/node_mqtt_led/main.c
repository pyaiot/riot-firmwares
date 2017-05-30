/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xtimer.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "board.h"

#include "periph/gpio.h"

#include "mqtt_common.h"
#include "mqtt_utils.h"

#define MQTT_PORT           (1883U)
#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)

#ifndef GATEWAY_ADDR
#define GATEWAY_ADDR "2001:660:3207:102::4"
#endif

#ifndef GATEWAY_PORT
#define GATEWAY_PORT 1885
#endif

#ifndef NODE_ID
#define NODE_ID "node_id_0"
#endif

#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define MAIN_QUEUE_SIZE       (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static char stack[THREAD_STACKSIZE_DEFAULT];

static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

typedef void (*mqtt_handler_t)(char *value);

typedef struct {
    const char *path;
    mqtt_handler_t handler;
} mqtt_resource_t;

void get_led(char *value) {
    sprintf(value, "%d", gpio_read(LED0_PIN) == 0);
}

static const mqtt_resource_t mqtt_resources[] = {
    {"board", get_board},
    {"mcu", get_mcu},
    {"os", get_os},
    {"name", get_name},
    {"led", get_led},
};

static const unsigned coap_resources_numof = sizeof(mqtt_resources) / sizeof(mqtt_resources[0]);

static void on_discover_pub(const emcute_topic_t *topic, void *data, size_t len)
{
    printf("### got publication for topic '%s' [%i] ###\n",
           topic->name, (int)topic->id);
    printf("%s", (char *)data);

    char payload[64] = {0};
    if (strcmp((char *)data, "resources") == 0) {
        sprintf(payload, "['board', 'mcu', 'os', 'name','led']");
        if (publish((uint8_t*)"node/resources", (uint8_t*)payload)) {
            return;
        }
    }
    else {
        for (unsigned i = 0; i < coap_resources_numof; ++i) {
            mqtt_resources[i].handler(payload);
            if (publish((uint8_t*)mqtt_resources[i].path, (uint8_t*)payload)) {
                continue;
            }
        }
    }
}

static void on_led_set_pub(const emcute_topic_t *topic, void *data, size_t len)
{
    char *in = (char *)data;

    printf("### got publication for topic '%s' [%i] ###\n",
           topic->name, (int)topic->id);
    for (size_t i = 0; i < len; i++) {
        printf("%c", in[i]);
    }

    if (strcmp((char *)data, "1") == 0) {
        LED0_ON;
    }
    else {
        LED0_OFF;
    }
    puts("");
}

static int initialize_mqtt_node(void)
{
    unsigned flags = EMCUTE_QOS_1;
    sock_udp_ep_t gw = {.family = AF_INET6, .port = GATEWAY_PORT};

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, GATEWAY_ADDR) == NULL) {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    if (emcute_con(&gw, true, NULL, NULL, 0, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n",
               GATEWAY_ADDR, (int)GATEWAY_PORT);
    }
    printf("Successfully connected to gateway at [%s]:%i\n",
           GATEWAY_ADDR, (int)GATEWAY_PORT);
    
    char gw_discover_topic[64] = { 0 };
    sprintf(gw_discover_topic, "gateway/%s/discover", NODE_ID);
    subscriptions[0].cb = on_discover_pub;
    strcpy(topics[0], gw_discover_topic);
    subscriptions[0].topic.name = topics[0];
    if (emcute_sub(&subscriptions[0], flags) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", gw_discover_topic);
        return 1;
    }
    printf("Now subscribed to %s\n", gw_discover_topic);

    char led_set_topic[64] = { 0 };
    sprintf(led_set_topic, "gateway/%s/led/set", NODE_ID);
    subscriptions[0].cb = on_led_set_pub;
    strcpy(topics[0], led_set_topic);
    subscriptions[0].topic.name = topics[0];
    if (emcute_sub(&subscriptions[0], flags) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", led_set_topic);
        return 1;
    }
    printf("Now subscribed to %s\n", led_set_topic);

    return 0;
}

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(MQTT_PORT, NODE_ID);
    return NULL;    /* should never be reached */
}

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

int main(void)
{
    puts("RIOT LED Node via MQTT-SN application");

    /* gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _netif_config(0, NULL);

    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    initialize_mqtt_node();

    return 0;
}
