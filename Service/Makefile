#include dirs
INC_DIRS = /usr/include/postgresql /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include /usr/include/libxml2 \
	   /usr/local/pgsql/include /usr/lib/arm-linux-gnueabihf/dbus-1.0/include /usr/include/nlohmann
LIB_DIRS = /usr/local/pgsql/lib

#General options
CC       = gcc
CPPFLAGS = -Wall -no-pie -std=c++11
LDFLAGS  = -lm -ljson-c -ldbus-1 -lcurl -lpthread -lz -lpq -lfftw3f -lstdc++ -lusb-1.0 -lmicrohttpd -lxml2 -lcctz

#Specific options
debug: CXXFLAGS += -O0 -g -DOZM_DEBUG -fno-exceptions
release: CXXFLAGS += -Os -Wall

#Source and targets files
TARGET_EXEC  = openZmeter
BUILD_DIR    = Tmp
MAINSOURCES  = $(shell find ./Source -maxdepth 2 -name "*.cpp" ! -type d -print)
MAINSOURCES += $(shell find ./Source -mindepth 3 -name "*.cpp" ! -type d -print)
RESOURCES    = $(shell find ./Resources ! -type d -print | sort)
RESTARGET    = $(BUILD_DIR)/ResourceFiles.o
RESTMP       = $(BUILD_DIR)/ResourceFiles
OBJS         = $(MAINSOURCES:%=$(BUILD_DIR)/%.o)
INC_DIRS    += $(shell find ./Source -type d)
INC_FLAGS   += $(addprefix -I,$(INC_DIRS))
LDFLAGS     += $(addprefix -L,$(LIB_DIRS))
CPPFLAGS    += $(INC_FLAGS)
debug: $(OBJS) $(RESTARGET)
	@echo "Linking debug target" $(TARGET_EXEC)
	@$(CC) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)
	
release: $(OBJS) $(WEBTARGET)
	@echo "Linking release target" $(TARGET_EXEC)
	@$(CC) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)

clean:
	@rm -rf $(BUILD_DIR)

package: $(BUILD_DIR)/$(TARGET_EXEC)
	@cp -r Package $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/Package/usr/sbin/$(TARGET_EXEC)
	@cp $(BUILD_DIR)/$(TARGET_EXEC) $(BUILD_DIR)/Package/usr/sbin/$(TARGET_EXEC)
	@dpkg-deb -b $(BUILD_DIR)/Package $(BUILD_DIR)/openZmeter.deb

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Bulding" $@
	@$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(RESTARGET): $(RESOURCES)
	@echo  "Building resource file"
	@mkdir -p $(BUILD_DIR)
	@rm -f $(RESTARGET) $(RESTMP)
	@printf "%s\000" $(words $(RESOURCES)) > $(RESTMP)
	@for FILE in $(RESOURCES); \
	do \
		echo "Append " $$FILE ; \
		printf '%s\000%d\000' `echo $$FILE | cut -d'/' -f3-` `stat --printf="%s" $$FILE` >> $(RESTMP) ; \
		cat $$FILE >> $(RESTMP) ; \
	done
	@cd $(BUILD_DIR); ld -r -b binary -o ResourceFiles.o ResourceFiles; cd ..
	@rm $(RESTMP)
