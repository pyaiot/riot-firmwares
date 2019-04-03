# name of your application
APPLICATION = node_mqtt_bmx280

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

DRIVER ?= bme280

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# include this to get the shell
USEMODULE += shell_commands
USEMODULE += shell
USEMODULE += ps

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

include $(CURDIR)/../Makefile.dep
include $(CURDIR)/../Makefile.include

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
