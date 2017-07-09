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
#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "board.h"

#include "periph/gpio.h"

#include "mqtt_common.h"
#include "mqtt_utils.h"

#define ENABLE_DEBUG   (1)
#include "debug.h"

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

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

#define EMCUTE_PRIO           (THREAD_PRIORITY_MAIN - 1)

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

static char payload[64] = {0};

static const mqtt_resource_t mqtt_resources[] = {
    {"board", get_board},
    {"mcu", get_mcu},
    {"os", get_os},
    {"name", get_name},
    {"led", get_led},
};

static const unsigned coap_resources_numof = sizeof(mqtt_resources) / sizeof(mqtt_resources[0]);

static void on_pub(const emcute_topic_t *topic, void *data, size_t len)
{
    DEBUG("[DEBUG] ### got publication for topic '%s' [%i] ###\n",
          topic->name, (int)topic->id);

    char topic_name[64] = {0};
    memset(payload, 0, sizeof(payload));
    if (strcmp(topic->name, "gateway/check") == 0) {
        DEBUG("[DEBUG] Replying to node/check publication\n");
        sprintf(payload, "{\"id\": %s}", NODE_ID);
        if (publish((uint8_t*)"node/check", (uint8_t*)payload)) {
            DEBUG("[ERROR] Failed to publish on node/check\n");
            return;
        }
    }
    else if (strcmp(topic->name, "gateway/1/discover") == 0) {
        DEBUG("[DEBUG] Replying to node/1/discover publication '%s'\n",
              (char*)data);
        if (strcmp((char *)data, "resources") == 0) {
            // sprintf(payload, "[\"board\", \"mcu\", \"os\", \"name\",\"led\"]");
            sprintf(payload, "[\"name\"]");
            memset(topic_name, 0, sizeof(topic_name));
            sprintf(topic_name, "node/%s/resources", NODE_ID);
            if (publish((uint8_t*)topic_name, (uint8_t*)payload)) {
                DEBUG("[ERROR] Failed to publish on node/%s/resources\n",
                       NODE_ID);
                return;
            }
        }
        else {
            for (unsigned i = 0; i < coap_resources_numof; ++i) {
                mqtt_resources[i].handler(payload);
                memset(topic_name, 0, sizeof(topic_name));
                sprintf(topic_name, "node/%s/%s", NODE_ID, mqtt_resources[i].path);
                if (publish((uint8_t*)topic_name, (uint8_t*)payload)) {
                    DEBUG("[ERROR] Failed to publish on %s\n", topic_name);
                    continue;
                }
            }
        }
    }
    else if (strcmp(topic->name, "gateway/1/led/set") == 0) {
        DEBUG("[DEBUG] Replying to node/1/led/set publication '%s'\n",
              (char*)data);
        if (strcmp((char *)data, "1") == 0) {
            LED0_ON;
        }
        else {
            LED0_OFF;
        }

        memset(payload, 0, sizeof(payload));
        sprintf(payload, "{\"value\": %s}", (char*)data);
        if (publish((uint8_t*)"node/check", (uint8_t*)payload)) {
            DEBUG("[ERROR] Failed to publish led status");
            return;
        }
    }
}

static int initialize_mqtt_node(void)
{
    unsigned flags = EMCUTE_QOS_1;
    sock_udp_ep_t gw = {.family = AF_INET6, .port = GATEWAY_PORT};

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, GATEWAY_ADDR) == NULL) {
        DEBUG("[ERROR] error parsing IPv6 address\n");
        return 1;
    }

    if (emcute_con(&gw, true, NULL, NULL, 0, 0) != EMCUTE_OK) {
        DEBUG("[ERROR] unable to connect to [%s]:%i\n",
               GATEWAY_ADDR, (int)GATEWAY_PORT);
        return 1;
    }
    DEBUG("[INFO] Successfully connected to gateway at [%s]:%i\n",
          GATEWAY_ADDR, (int)GATEWAY_PORT);

    char gw_discover_topic[64] = { 0 };
    sprintf(gw_discover_topic, "gateway/%s/discover", NODE_ID);
    subscriptions[0].cb = on_pub;
    strcpy(topics[0], gw_discover_topic);
    subscriptions[0].topic.name = topics[0];
    if (emcute_sub(&subscriptions[0], flags) != EMCUTE_OK) {
        DEBUG("[ERROR] unable to subscribe to %s\n", gw_discover_topic);
        return 1;
    }
    DEBUG("[INFO] Now subscribed to %s\n", gw_discover_topic);

    char node_check_topic[64] = { 0 };
    sprintf(node_check_topic, "gateway/check");
    subscriptions[1].cb = on_pub;
    strcpy(topics[1], node_check_topic);
    subscriptions[1].topic.name = topics[1];
    if (emcute_sub(&subscriptions[1], flags) != EMCUTE_OK) {
        DEBUG("[ERROR] unable to subscribe to %s\n", node_check_topic);
        return 1;
    }
    DEBUG("[INFO] Now subscribed to %s\n", node_check_topic);

    char led_set_topic[64] = { 0 };
    sprintf(led_set_topic, "gateway/%s/led/set", NODE_ID);
    subscriptions[2].cb = on_pub;
    strcpy(topics[2], led_set_topic);
    subscriptions[2].topic.name = topics[2];
    if (emcute_sub(&subscriptions[2], flags) != EMCUTE_OK) {
        DEBUG("[ERROR] unable to subscribe to %s\n", led_set_topic);
        return 1;
    }
    DEBUG("[INFO] Now subscribed to %s\n", led_set_topic);

    memset(payload, 0, sizeof(payload));
    sprintf(payload, "{\"id\": %s}", NODE_ID);
    if (publish((uint8_t*)"node/check", (uint8_t*)payload)) {
        DEBUG("[ERROR] Failed to publish led status\n");
        return 1;
    }

    DEBUG("[INFO] MQTT node initialized with success\n");

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

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
