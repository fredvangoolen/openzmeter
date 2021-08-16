#Executable related components
EXECUTABLE_TARGET = Tmp/Firmware
EXECUTABLE_SOURCE = $(shell find . -name "*.c" ! -type d -print)
CFLAGS += -D_GNU_SOURCE
CFLAGS += -ggdb -Wall -O2 -std=c99 -flto -mcpu=cortex-m4 -mthumb -mthumb-interwork -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fstack-usage -fsingle-precision-constant -fno-exceptions --specs=nosys.specs
LDFLAGS += -g -T CMSIS/stm32f303xb.ld -flto -nostdlib -lc -Wl,-Map=$(EXECUTABLE_TARGET:%=%.map)

CFLAGS += -I CMSIS
CC=arm-none-eabi-gcc
EXECUTABLE_OBJS	+= $(EXECUTABLE_SOURCE:%=Tmp/%.o)
#=======================================================================================================================
default: $(EXECUTABLE_TARGET)

clean:
	@rm -rf Tmp

$(EXECUTABLE_TARGET): $(EXECUTABLE_OBJS)
	@echo "Linking target" $(TARGET_EXEC)
	@$(CC) $(CFLAGS) $(EXECUTABLE_OBJS) -o $(EXECUTABLE_TARGET) $(LDFLAGS)
	@arm-none-eabi-size -A $(EXECUTABLE_TARGET)
	@arm-none-eabi-objdump -S -x $(EXECUTABLE_TARGET) > Tmp/Firmware.lst

Tmp/%.c.o: %.c
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@

Tmp/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@

Tmp/%.s.o: %.s
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@