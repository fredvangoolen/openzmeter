#Default build dir
TARGET = $(notdir $(CONFIG))

#WEB related components
WEB_TARGET = Tmp/openZmeter.js
WEB_SOURCE = $(shell find WEB -type f ! -path "WEB/node_modules/*")

#Resources related components
RESOURCES_TARGET = Tmp/$(TARGET)/ResourceFiles.o
RESOURCES_SOURCE = $(shell find Resources ! -type d -print | sort)

#Executable related components
EXECUTABLE_TARGET = Tmp/$(TARGET)/openZmeter
EXECUTABLE_SOURCE = $(shell find Source -name "*.cpp" ! -type d -print)
CFLAGS  += -DCPPHTTPLIB_USE_POLL -DCPPHTTPLIB_TCP_NODELAY -DCPPHTTPLIB_ZLIB_SUPPORT -DCPPHTTPLIB_OPENSSL_SUPPORT -D__BUILD__="\"$(BUILD_NAME)_$(BUILD_DATE)\"" \
           -I Source -I Source/Devices -I Source/Analyzers -I Source/Analyzers/AnalyzerSoft -I Source/Analyzers/AnalyzerRemote -I Source/Tariffs -I Source/Database -I Source/HTTPServer -I Source/System -I Source/Tools -I Libs -I Libs/mqtt-c \
					 -fsingle-precision-constant -Wno-psabi -Wall
LDFLAGS += -lstdc++ -lpq -lz -lpthread -lm -lssl -lcrypto -luuid -lfftw3f -lusb-1.0

#Add libs (cctz, nlohmann, muparser, httplib, mqtt-c)
EXECUTABLE_SOURCE += $(shell find Libs/cctz -name "*.cpp" ! -type d -print)
EXECUTABLE_SOURCE += $(shell find Libs/muParser -name "*.cpp" ! -type d -print)
EXECUTABLE_SOURCE += $(shell find Libs/mqtt-c -name "*.c" ! -type d -print)

#Fill date and version if not provided
BUILD_DATE ?= $(shell date +%Y%m%d.%H%M)
BUILD_NAME ?= "custom"

#Default flags and programs
CC  ?= gcc
LD  ?= ld

#Objects and depends
EXECUTABLE_OBJS	 = $(EXECUTABLE_SOURCE:%=Tmp/$(TARGET)/%.o)
DEPENDS          = $(EXECUTABLE_OBJS:%.o=%.d)
#=======================================================================================================================
default: READCONFIG $(EXECUTABLE_TARGET)

clean-c:
	@rm -rf Tmp/$(TARGET)/Libs Tmp/$(TARGET)/openZmeter Tmp/$(TARGET)/*.o Tmp/$(TARGET)/Source
clean:
	@rm -rf Tmp/$(TARGET) Tmp/*.js

READCONFIG:
	@echo "Building version '$(BUILD_NAME)_$(BUILD_DATE)'"
ifdef CONFIG
	@mkdir -p Tmp/$(TARGET)
	@echo "Reading configuration from '$(CONFIG)'"
include $(CONFIG)
else
	@echo "Use make CONFIG={config_file}"
	@false
endif
ifeq ($(DEBUG), true)
	CFLAGS += -O0 -g -ggdb3
else
	CFLAGS   += -Os -flto -fwhole-program
	LDFLAGS  += -flto
endif
ifeq ($(OS_TARGET), OPENWRT)
	CFLAGS += -DOPENWRT
	LDFLAGS += -ldbus-1
endif
ifeq ($(OS_TARGET), DEBIAN)
	CFLAGS += -DDEBIAN
	LDFLAGS += -ldbus-1
endif

# Build steps
$(WEB_TARGET): $(WEB_SOURCE)
	@echo "Building WEB interface"
	@rm -f $(WEB_TARGET)
	@if [ ! -d "WEB/node_modules" ]; then cd WEB; npm install; fi
	@cd WEB; npm run --silent build

$(RESOURCES_TARGET): $(RESOURCES_SOURCE) $(WEB_TARGET)
	@echo  "Building resource file"
	@rm -f $(RESOURCES_TARGET) Tmp/ResourceFiles
	@printf "%s\000" $(words $(RESOURCES_SOURCE) $(WEB_TARGET)) > Tmp/ResourceFiles
	@for FILE in $(RESOURCES_SOURCE); \
	do \
		echo "  Append " $$FILE ; \
		printf '%s\000%d\000' `echo $$FILE | cut -d'/' -f2-` `stat --printf="%s" $$FILE` >> Tmp/ResourceFiles ; \
		cat $$FILE >> Tmp/ResourceFiles ; \
	done
	@echo "  Append " $(WEB_TARGET) ; \
		printf 'WEB/%s\000%d\000' `echo $(WEB_TARGET) | cut -d'/' -f2-` `stat --printf="%s" $(WEB_TARGET)` >> Tmp/ResourceFiles ; \
		cat $(WEB_TARGET) >> Tmp/ResourceFiles ;
	@cd Tmp; $(LD) -s -r -b binary -o $(TARGET)/ResourceFiles.o ResourceFiles
	@rm -f Tmp/ResourceFiles

$(EXECUTABLE_TARGET): $(RESOURCES_TARGET) $(EXECUTABLE_OBJS)
	@echo "Linking target" $(TARGET_EXEC)
	@$(CC) $^ -o $(EXECUTABLE_TARGET) $(LDFLAGS)

-include $(DEPENDS)

# File types
Tmp/$(TARGET)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Building" $@
	@$(CC) $(CFLAGS) -MMD -std=c++17 -c $< -o $@

Tmp/$(TARGET)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo "Building" $@
	@$(CC) $(CFLAGS) -c $< -o $@