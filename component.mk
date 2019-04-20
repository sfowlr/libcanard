
DSDLC_PATH := $(COMPONENT_PATH)/dsdl_compiler/libcanard_dsdlc
DSDL_PATH := $(COMPONENT_PATH)/dsdl_compiler/pyuavcan/uavcan/dsdl_files/uavcan
DSDL_HEADERS_PATH := $(COMPONENT_PATH)/dsdl_compiler/dsdlc_generated

COMPONENT_ADD_INCLUDEDIRS := . ./drivers/esp $(DSDL_HEADERS_PATH)
COMPONENT_SRCDIRS := . ./drivers/esp
CXXFLAGS += -fno-rtti

CFLAGS += -D LOG_LOCAL_LEVEL=ESP_LOG_VERBOSE

COMPONENT_EXTRA_CLEAN := $(shell find $(DSDL_HEADERS_PATH) -type f)

canard.o: dsdl_headers

dsdl_headers:
	$(PYTHON) $(DSDLC_PATH) -v --header_only --outdir $(DSDL_HEADERS_PATH) $(DSDL_PATH)