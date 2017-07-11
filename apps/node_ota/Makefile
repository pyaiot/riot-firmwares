# name of your application
APPLICATION = node_ota

# If no BOARD is found in the environment, use this default:
BOARD ?= iotlab-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# OTA definitions
USEMODULE += ota_update

FW_SLOT = 1
VERSION = 0x1
APPID = 0xBBA

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_sock_udp

USEPKG += nanocoap
USEMODULE += gcoap
USEMODULE += posix

USEMODULE += xtimer
USEMODULE += fmt

# include this to get the shell
USEMODULE += shell_commands
USEMODULE += shell
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6

FEATURES_REQUIRED += periph_gpio

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Required modules
USEMODULE += coap_common
DIRS += $(CURDIR)/../../modules/coap_common
INCLUDES += -I$(CURDIR)/../../modules/coap_common

USEMODULE += coap_utils
DIRS += $(CURDIR)/../../modules/coap_utils
INCLUDES += -I$(CURDIR)/../../modules/coap_utils

USEMODULE += coap_ota
DIRS += $(CURDIR)/../../modules/coap_ota
INCLUDES += -I$(CURDIR)/../../modules/coap_ota

ifneq (1, $(FW_SLOT))
USEMODULE += coap_led
DIRS += $(CURDIR)/../../modules/coap_led
INCLUDES += -I$(CURDIR)/../../modules/coap_led
else
USEMODULE += coap_imu
DIRS += $(CURDIR)/../../modules/coap_imu
INCLUDES += -I$(CURDIR)/../../modules/coap_imu
endif

# Application specific definitions and includes
APPLICATION_NAME ?= Node\ OTA
# CoAP broker server information
BROKER_ADDR ?= 2001:660:3207:102::4
BROKER_PORT ?= 5683

include $(RIOTBASE)/Makefile.include

CFLAGS += -DBROKER_ADDR=\"$(BROKER_ADDR)\"
CFLAGS += -DBROKER_PORT=$(BROKER_PORT)
CFLAGS += -DAPPLICATION_NAME=\"$(APPLICATION_NAME)\"
CFLAGS += -DVERSION=$(FIRMWARE_VERSION)
CFLAGS += -DAPPID=$(FIRMWARE_APPID)

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
