CONTIKI_PROJECT = flow-sensor 

all: $(CONTIKI_PROJECT)

CONTIKI ?= ../../../..
PROJECT_DIR ?= ../..

cleanall: clean
	@rm -f *.${TARGET}
	@rm -fr build/upload-${TARGET}


include $(CONTIKI)/Makefile.dir-variables

include Makefile.config

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
