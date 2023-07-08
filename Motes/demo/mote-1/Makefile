CONTIKI_PROJECT = mote-1

all: $(CONTIKI_PROJECT)

CONTIKI ?= ../../../..
PROJECT_DIR ?= ../..

include $(CONTIKI)/Makefile.dir-variables

# Include custom modules
MODULES_REL += $(PROJECT_DIR)/lib/common
MODULES_REL += $(PROJECT_DIR)/lib/resources
MODULES_REL += $(PROJECT_DIR)/lib/webserver

# CoAP implementation
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
MODULES_REL += $(PROJECT_DIR)/lib/resources

# Border router
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router

# JSON library (from contiki-ng)
MODULES += $(CONTIKI_NG_LIB_DIR)/json

# Platform specific module
-include $(CONTIKI)/Makefile.identify-target
MODULES_REL += arch/platform/$(TARGET)

# Include makefile
include $(CONTIKI)/Makefile.include
