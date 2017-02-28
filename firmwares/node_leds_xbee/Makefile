# name of your application
APPLICATION = node_leds_xbee

# If no BOARD is found in the environment, use this default:
BOARD = arduino-zero

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

USEMODULE += xbee
USEMODULE += gnrc_netdev2
USEMODULE += gnrc_netif
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_udp
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_conn_udp
USEMODULE += shell_commands

USEPKG += microcoap
CFLAGS += -DMICROCOAP_DEBUG

XBEE_UART ?= "1"
CFLAGS += -DXBEE_PARAM_UART=$(XBEE_UART)

## add current directory to the include path. Putting it in CFLAGS will make
## it go to the beginning, before the standard includes.
## That way xbee_params.h get's included and auto configuration can pick it up.
CFLAGS += -I$(CURDIR)

# CoAP broker server information
BROKER_ADDR ?= 2001:660:3207:102::4

CFLAGS += -DBROKER_ADDR=\"$(BROKER_ADDR)\"

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
#CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

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
