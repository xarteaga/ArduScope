#
#	Made by Xavier Arteaga
#	xavie.arteaga@gmail.com
#

# Arduino Connection
###############################################################################
BAUD_RATE = 115200
COM_PORT = "/dev/ttyACM0"

# Build PATH
###############################################################################
BUILD_PATH = ./build

# Compilation and upload options
###############################################################################
ARDUINO_OPT = -w -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -finline-functions -funswitch-loops -DARDUINO=100 -Wl,--gc-sections
ARDUINO_CHIP = atmega328p
ARDUINO_RATE = 115200	# serial bit rate
ARDUINO_FCPU = 16000000L # CPU frequency in Hz

# Arduino core headers and sources paths
###############################################################################
ARDUINO_CORE_PATH = /usr/share/arduino/hardware/arduino/cores/arduino/
ARDUINO_PINS_PATH = /usr/share/arduino/hardware/arduino/variants/standard/
ARDUINO_LIBS_PATH = $(shell find libraries/* -type d -print)


# Arduino core headers and sources names
###############################################################################
ARDUINO_CORES = $(wildcard $(ARDUINO_CORE_PATH)/*.c) $(wildcard $(ARDUINO_CORE_PATH)/*/*.c)
ARDUINO_CORES_XX = $(filter-out /usr/share/arduino/hardware/arduino/cores/arduino//main.cpp,$(wildcard $(ARDUINO_CORE_PATH)/*.cpp))

ARDUINO_LIBS = $(shell find libraries/ -name \*.c)
ARDUINO_LIBS_XX = $(shell find libraries/ -name \*.cpp)
ARDUINO_LIBS_INC = $(foreach d, $(ARDUINO_LIBS_PATH), -I$d)

# Arduino Compilers
###############################################################################
CC = avr-gcc
CXX = avr-g++
AVR_DUDE_LINUX = avrdude -C /usr/share/arduino/hardware/tools/avrdude.conf

# Flags
###############################################################################
LDFLAGS = $(ARDUINO_OPT) -mmcu=$(ARDUINO_CHIP) -DF_CPU=$(ARDUINO_FCPU) -I$(ARDUINO_CORE_PATH) -I$(ARDUINO_PINS_PATH) $(ARDUINO_LIBS_INC)
# User Sources
###############################################################################
SOURCES = main.cpp sha1.cpp Base64.cpp

# Binary loadable files...
###############################################################################
EXECUTABLE=prog.elf
BINARY=$(EXECUTABLE:.elf=.hex)

# Rules
###############################################################################
all: $(EXECUTABLE)

$(EXECUTABLE):
	$(CXX) $(LDFLAGS) -o $(BUILD_PATH)/$@ $(ARDUINO_CORES_XX) $(ARDUINO_CORES) $(ARDUINO_LIBS) $(ARDUINO_LIBS_XX) $(SOURCES)

core: $(CORE_OBJECTS)

load:
	#avr-objcopy -O ihex -R .eeprom $(BUILD_PATH)/$(EXECUTABLE) $(BUILD_PATH)/$(BINARY)
	#avrdude -F -V -c arduino -p $(ARDUINO_CHIP) -P $(SERIALPORT) -b $(ARDUINO_RATE) -U flash:w:$(BUILD_PATH)/$(BINARY)
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $(BUILD_PATH)/$(EXECUTABLE) $(BUILD_PATH)/$(BINARY).eep
	avr-objcopy -O ihex -R .eeprom $(BUILD_PATH)/$(EXECUTABLE) $(BUILD_PATH)/$(BINARY) 
	avr-size --mcu=${ARDUINO_CHIP} -C $(BUILD_PATH)/$(BINARY)
	$(AVR_DUDE_LINUX) -v -p${ARDUINO_CHIP} -carduino -P ${COM_PORT} -b${BAUD_RATE} -D -Uflash:w:$(BUILD_PATH)/$(BINARY):i

clean:
	rm -f $(BUILD_PATH)/*.o $(BUILD_PATH)/$(EXECUTABLE) $(BUILD_PATH)/$(BINARY)

