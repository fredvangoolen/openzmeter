#Executable related components
EXECUTABLE_TARGET = Tmp/Firmware
EXECUTABLE_SOURCE = main.c
CFLAGS += -Wno-psabi -Wall -std=c++11 -I Source -I Source/Devices -I Source/Analyzers -I Source/Tariffs 
LDFLAGS += -lstdc++ -lpq -lmicrohttpd -lz

#Add libs (cctz, nlohmann)
#EXECUTABLE_SOURCE += $(shell find Libs/cctz -name "*.cpp" ! -type d -print)
#CFLAGS += -I Libs -I Source/Support
CC?=gcc
LD?=ld
EXECUTABLE_OBJS	+= $(EXECUTABLE_SOURCE:%=Tmp/%.o)
#=======================================================================================================================
default: READCONFIG $(EXECUTABLE_TARGET)

clean:
	@rm -rf Tmp

READCONFIG:
ifdef CONFIG
	@mkdir -p Tmp
	@echo "Reading configuration from '$(CONFIG)'"
include $(CONFIG)
else
	@echo "Use make CONFIG={config_file}"
	@false
endif
ifeq ($(DEBUG), true)
	CFLAGS += -O0 -g -g3 -ggdb 
else
	CFLAGS += -Os -Wall -fno-rtti -fopt-info-vec -s
endif

$(EXECUTABLE_TARGET): $(EXECUTABLE_OBJS) $(RESOURCES_TARGET)
	@echo "Linking target" $(TARGET_EXEC)
	$(CC)  $(EXECUTABLE_OBJS) $(RESOURCES_TARGET) -o $(EXECUTABLE_TARGET) $(LDFLAGS)

Tmp/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Building" $@
	@$(CC) $(CFLAGS) -c $< -o $@
