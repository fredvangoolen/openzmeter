#Executable related components
EXECUTABLE_TARGET = Tmp/Firmware
EXECUTABLE_SOURCE = Main.c
CFLAGS += -Wall -std=c99 -mcpu=cortex-m0 -mthumb -g -g2 -Os
LDFLAGS += -TSTM32F042K6.ld -Wl,--gc-sections -nostdlib -Wl,-Map=$(EXECUTABLE_TARGET:%=%.map)

#Add CMSIS
EXECUTABLE_SOURCE += $(shell find CMSIS -name "*.s" ! -type d -print) $(shell find CMSIS -name "*.c" ! -type d -print)
CFLAGS += -I CMSIS
CC=arm-none-eabi-gcc
EXECUTABLE_OBJS	+= $(EXECUTABLE_SOURCE:%=Tmp/%.o)
#=======================================================================================================================
default: $(EXECUTABLE_TARGET)

clean:
	@rm -rf Tmp

$(EXECUTABLE_TARGET): $(EXECUTABLE_OBJS)
	@echo "Linking target" $(TARGET_EXEC)
	@$(CC)  $(EXECUTABLE_OBJS) -o $(EXECUTABLE_TARGET) $(LDFLAGS)
	@arm-none-eabi-objcopy -O binary $(EXECUTABLE_TARGET) Tmp/Image.bin
	@arm-none-eabi-size $(EXECUTABLE_TARGET)

Tmp/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@

Tmp/%.s.o: %.s
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@