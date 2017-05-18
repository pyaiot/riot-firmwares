# name of your application
APPLICATION = dashboard_riot_a8_m3

# Use IoT-LAB board:
BOARD = iotlab-a8-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
USEMODULE += printf_float
USEMODULE += gnrc_sock_udp
USEMODULE += lsm303dlhc

USEPKG += nanocoap
USEMODULE += gcoap
USEMODULE += posix

# include this for printing IP addresses
USEMODULE += shell_commands

USEMODULE += coap_common
USEMODULE += coap_utils
USEMODULE += coap_led
USEMODULE += coap_position
USEMODULE += coap_iotlab_a8_m3

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
APPLICATION_NAME ?= \"IoT-Lab\ A8\ Node\"
NODE_POSITION ?= {"lat":48.714784,"lng":2.205502}
# CoAP broker server information
BROKER_ADDR ?= 2001:660:3207:102::4
BROKER_PORT ?= 5683

DIRS += $(CURDIR)/../../modules/coap_common
INCLUDES += -I$(CURDIR)/../../modules/coap_common

DIRS += $(CURDIR)/../../modules/coap_utils
INCLUDES += -I$(CURDIR)/../../modules/coap_utils

DIRS += $(CURDIR)/../../modules/coap_position
INCLUDES += -I$(CURDIR)/../../modules/coap_position

DIRS += $(CURDIR)/../../modules/coap_led
INCLUDES += -I$(CURDIR)/../../modules/coap_led

DIRS += $(CURDIR)/../../modules/coap_iotlab_a8_m3
INCLUDES += -I$(CURDIR)/../../modules/coap_iotlab_a8_m3

include $(RIOTBASE)/Makefile.include

CFLAGS += -DBROKER_ADDR=\"$(BROKER_ADDR)\"
CFLAGS += -DBROKER_PORT=$(BROKER_PORT)
CFLAGS += -DAPPLICATION_NAME=$(APPLICATION_NAME)
CFLAGS += -DNODE_POSITION=\"$(NODE_POSITION)\"

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
