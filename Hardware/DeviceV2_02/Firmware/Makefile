#Executable related components
EXECUTABLE_TARGET = Tmp/Firmware
EXECUTABLE_SOURCE = Main.c System.c USB.c
LIBSPEC=-L /usr/lib/gcc/arm-none-eabi/5.4.1/armv6-m
CFLAGS += -Wall -std=c99 -mcpu=cortex-m0 -mthumb -march=armv6-m -mfloat-abi=soft -mlong-calls -g -O1
LDFLAGS += -T STM32F042K6.ld -nostdlib -lgcc -Wl,--cref -Wl,-Map=$(EXECUTABLE_TARGET:%=%.map)

#Add CMSIS
#EXECUTABLE_SOURCE += $(shell find CMSIS -name "*.s" ! -type d -print) $(shell find CMSIS -name "*.c" ! -type d -print)
CFLAGS += -I CMSIS
CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
EXECUTABLE_OBJS	+= $(EXECUTABLE_SOURCE:%=Tmp/%.o)
#=======================================================================================================================
default: $(EXECUTABLE_TARGET)

clean:
	@rm -rf Tmp

$(EXECUTABLE_TARGET): $(EXECUTABLE_OBJS)
	@echo "Linking target" $(TARGET_EXEC)
	@$(CC) $(EXECUTABLE_OBJS) $(LIBSPEC) -o $(EXECUTABLE_TARGET) $(LDFLAGS)
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