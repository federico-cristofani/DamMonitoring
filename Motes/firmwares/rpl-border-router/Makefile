CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)

CONTIKI ?= ../../..
PROJECT_DIR ?= ../..

# Include RPL BR module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router

# Include webserver module
MODULES_REL += $(PROJECT_DIR)/lib/webserver

# Include optional target-specific module
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.include
