#include dirs
INC_DIRS = /usr/include/postgresql /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include /usr/include/libxml2 \
	   /usr/local/pgsql/include /usr/lib/arm-linux-gnueabihf/dbus-1.0/include /usr/include/nlohmann
LIB_DIRS = /usr/local/pgsql/lib

#General options
CC       = gcc
LD       = ld
CPPFLAGS += -Wall -std=c++11 -march=native
LDFLAGS  += -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic -lmicrohttpd -lusb-1.0 -ldbus-1 -lpthread -lpq -ldbus-1 -lz -lm -lfftw3f

#Specific options
debug: CXXFLAGS += -O0 -g -DOZM_DEBUG
release: CXXFLAGS += -Os -s -Wall -flto

#Source and targets files
TARGET_EXEC  = openZmeter
BUILD_DIR    = Tmp
MAINSOURCES  = $(shell find ./Source -maxdepth 2 -name "*.cpp" ! -type d -print) $(shell find ./Libs -name "*.cpp" -o -name "*.cc" ! -type d -print)
MAINSOURCES += $(shell find ./Source -mindepth 3 -name "*.cpp" ! -type d -print)
RESOURCES    = $(shell find ./Resources ! -type d -print | sort)
RESTARGET    = $(BUILD_DIR)/ResourceFiles.o
RESTMP       = $(BUILD_DIR)/ResourceFiles
OBJS         = $(MAINSOURCES:%=$(BUILD_DIR)/%.o)
INC_DIRS    += $(shell find ./Source -type d) $(shell find ./Libs -type d)
INC_FLAGS   += $(addprefix -I,$(INC_DIRS))
LDFLAGS     += $(addprefix -L,$(LIB_DIRS))
CPPFLAGS    += $(INC_FLAGS)
debug: $(OBJS) $(RESTARGET)
	@echo "Linking debug target" $(TARGET_EXEC)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)
	
release: $(OBJS) $(RESTARGET)
	@echo "Linking release target" $(TARGET_EXEC)
	@$(CC) $(CPPFLAGS) $(CXXFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)

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
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	@echo "Bulding" $@
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

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
	@cd $(BUILD_DIR); $(LD) -r -b binary -o ResourceFiles.o ResourceFiles; cd ..
	@rm $(RESTMP)
