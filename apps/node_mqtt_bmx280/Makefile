# name of your application
APPLICATION = node_mqtt_bmx280

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

DRIVER ?= bme280

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_ipv6_router_default
# Include MQTT-SN
USEMODULE += emcute
USEMODULE += posix

# include this to get the shell
USEMODULE += shell_commands
USEMODULE += shell
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6

USEMODULE += mqtt_utils
USEMODULE += mqtt_common
USEMODULE += mqtt_bmx280
USEMODULE += $(DRIVER)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
NODE_ID ?= "mqtt-2"
APPLICATION_NAME ?= "Node\ BME280\ MQTT-SN"
# CoAP broker server information
GATEWAY_ADDR ?= fd00:abad:1e:102::1
GATEWAY_PORT ?= 1885

DIRS += $(CURDIR)/../../modules/mqtt_common
INCLUDES += -I$(CURDIR)/../../modules/mqtt_common

DIRS += $(CURDIR)/../../modules/mqtt_bmx280
INCLUDES += -I$(CURDIR)/../../modules/mqtt_bmx280

DIRS += $(CURDIR)/../../modules/mqtt_utils
INCLUDES += -I$(CURDIR)/../../modules/mqtt_utils

include $(RIOTBASE)/Makefile.include

CFLAGS += -DNODE_ID="\"$(NODE_ID)\""
CFLAGS += -DGATEWAY_ADDR=\"$(GATEWAY_ADDR)\"
CFLAGS += -DGATEWAY_PORT=$(GATEWAY_PORT)
CFLAGS += -DAPPLICATION_NAME="\"$(APPLICATION_NAME)\""

# Set a custom channel if needed
ifneq (,$(filter cc110x,$(USEMODULE)))          # radio is cc110x sub-GHz
  DEFAULT_CHANNEL ?= 0
  CFLAGS += -DCC110X_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
else
  ifneq (,$(filter at86rf212b,$(USEMODULE)))    # radio is IEEE 802.15.4 sub-GHz
    DEFAULT_CHANNEL ?= 5
    FLAGS += -DIEEE802154_DEFAULT_SUBGHZ_CHANNEL=$(DEFAULT_CHANNEL)
  else                                          # radio is IEEE 802.15.4 2.4 GHz
    DEFAULT_CHANNEL ?= 26
    CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
  endif
endif