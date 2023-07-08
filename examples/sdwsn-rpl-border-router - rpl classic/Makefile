CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)
CONTIKI = ../..

# The BR is either native or embedded, and in the latter case must support SLIP
PLATFORMS_EXCLUDE = z1 sky 

# Include RPL BR module and coap
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC

# Include webserver module and coap resources

MODULES_REL += webserver
MODULES_REL += ./resources

# Include optional target-specific module
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.include
