CONTIKI_PROJECT = coap-example-server
all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1

# Include CoAP resources
MODULES_REL += ./resources
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
CONTIKI=../..

# Include the CoAP implementation

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

#SDWSN = 1

include $(CONTIKI)/Makefile.include
