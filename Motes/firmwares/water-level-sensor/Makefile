CONTIKI_PROJECT = water-level-sensor 

all: $(CONTIKI_PROJECT)

CONTIKI ?= ../../../..
PROJECT_DIR ?= ../..

include $(CONTIKI)/Makefile.dir-variables

# Include custom modules
MODULES_REL += $(PROJECT_DIR)/lib/common

# MQTT client
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/mqtt
MODULES_REL += $(PROJECT_DIR)/lib/mqtt-client

# JSON library (from contiki-ng)
MODULES += $(CONTIKI_NG_LIB_DIR)/json

# Platform specific module
-include $(CONTIKI)/Makefile.identify-target
MODULES_REL += arch/platform/$(TARGET)

# Include makefile
include $(CONTIKI)/Makefile.include
