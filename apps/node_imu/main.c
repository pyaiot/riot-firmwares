/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "shell.h"
#include "nanocoap.h"
#include "net/gcoap.h"

/* RIOT firmware libraries */
#include "coap_common.h"
#include "coap_imu.h"

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

#define MAIN_QUEUE_SIZE       (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/board", COAP_GET, board_handler },
    { "/imu", COAP_GET, coap_imu_handler },
    { "/mcu", COAP_GET, mcu_handler },
    { "/name", COAP_GET, name_handler },
    { "/os", COAP_GET, os_handler },
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};

int main(void)
{
    puts("RIOT IMU Node application");

    /* gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _netif_config(0, NULL);

    /* start coap server loop */
    gcoap_register_listener(&_listener);
    init_beacon_sender();
    init_imu_sender();

    LED0_TOGGLE;
    LED1_TOGGLE;
    LED2_TOGGLE;

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
