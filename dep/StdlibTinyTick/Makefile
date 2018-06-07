###############################################################################
# Makefile for the project StdlibTinyTick
###############################################################################

## General Flags
PROJECT = StdlibTinyTick
MCU = attiny461a
TARGET = StdlibTinyTick.elf
CC = avr-gcc

OBJECTS = bin/main.o bin/softserial.o
DEPFILES:= $(patsubst %.o, %.o.d, $(OBJECTS))
LINKONLYOBJECTS = 

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT bin/$(*F).o -MF bin/$(@F).d 

## Linker flags
LDFLAGS = $(COMMON)

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Build
all: bin/$(TARGET) bin/$(PROJECT).hex bin/$(PROJECT).eep bin/$(PROJECT).lss bin/$(PROJECT).symbols

bin/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c  $<

##Link
bin/$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o bin/$(TARGET)

bin/%.hex: bin/$(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

bin/%.eep: bin/$(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

bin/%.lss: bin/$(TARGET)
	avr-objdump -h -S $< > $@
	
bin/%.symbols: bin/$(TARGET)
	avr-objdump -t $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) bin/$(PROJECT).elf bin/*.d bin/$(PROJECT).hex bin/$(PROJECT).eep bin/$(PROJECT).lss bin/$(PROJECT).map

-include $(DEPFILES)

