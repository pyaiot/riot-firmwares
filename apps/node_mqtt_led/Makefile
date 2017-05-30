# name of your application
APPLICATION = node_mqtt_led

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_ipv6_default
# Include MQTT-SN
USEMODULE += emcute

# include this for printing IP addresses
USEMODULE += shell_commands

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
NODE_ID ?= node_mqtt_led_1
APPLICATION_NAME ?= Node\ LED\ MQTT
NODE_POSITION ?= {\"lat\":48.714784,\"lng\":2.205502}
# CoAP broker server information
GATEWAY_ADDR ?= 2001:660:3207:102::4
GATEWAY_PORT ?= 1885

include $(RIOTBASE)/Makefile.include

CFLAGS += -DNODE_ID=\"$(NODE_ID)\"
CFLAGS += -DGATEWAY_ADDR=\"$(GATEWAY_ADDR)\"
CFLAGS += -DGATEWAY_PORT=$(GATEWAY_PORT)
CFLAGS += -DAPPLICATION_NAME=\"$(APPLICATION_NAME)\"
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
