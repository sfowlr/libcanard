COMPONENT_ADD_INCLUDEDIRS := . ./drivers/esp ./dsdl_compiler/dsdlc_generated
COMPONENT_SRCDIRS := . ./drivers/esp
CXXFLAGS += -fno-rtti

CFLAGS += -D LOG_LOCAL_LEVEL=ESP_LOG_VERBOSE