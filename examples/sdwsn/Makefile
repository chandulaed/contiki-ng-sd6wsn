CONTIKI_PROJECT = coap-example-server
all: $(CONTIKI_PROJECT)

# Do not try to build on Sky because of code size limitation
PLATFORMS_EXCLUDE = sky z1

# Include CoAP resources
MODULES_REL += ./resources

CONTIKI=../..

# Include the CoAP implementation

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
MAKE_WITH_DTLS=1
MAKE_COAP_DTLS_KEYSTORE=MAKE_COAP_DTLS_KEYSTORE_SIMPLE
#SDWSN = 1

include $(CONTIKI)/Makefile.include
