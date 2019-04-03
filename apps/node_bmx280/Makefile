# name of your application
APPLICATION = node_bmx280

# If no BOARD is found in the environment, use this default:
BOARD = samr21-xpro

DRIVER ?= bme280

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Application specific modules
USEMODULE += printf_float
USEMODULE += $(DRIVER)

# include this to get the shell
USEMODULE += shell_commands
USEMODULE += shell
USEMODULE += ps

USEMODULE += coap_common
USEMODULE += coap_utils
USEMODULE += coap_position
USEMODULE += coap_bmx280

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
APPLICATION_NAME ?= "Weather\ Sensor"
# CoAP broker server information
BROKER_ADDR ?= 2001:660:3207:102::4
BROKER_PORT ?= 5683

include $(CURDIR)/../Makefile.dep
include $(CURDIR)/../Makefile.include

include $(RIOTBASE)/Makefile.include

CFLAGS += -DBROKER_ADDR=\"$(BROKER_ADDR)\"
CFLAGS += -DBROKER_PORT=$(BROKER_PORT)
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
