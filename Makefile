# Microcontroller type
MCU=atmega328p
# Microcontroller freqency
F_CPU=16000000
CC=avr-gcc
CXX=avr-c++
MAKE_IT_SMALL_FLAGS=-mcall-prologues -ffunction-sections -fdata-sections -Wl,--gc-sections
COMMON_FLAGS=-g -Os $(MAKE_IT_SMALL_FLAGS) -Wall -mmcu=$(MCU) -I. -I./arduino -I./SPI -DF_CPU=16000000
CXXFLAGS=$(COMMON_FLAGS)
CFLAGS=$(COMMON_FLAGS) -std=c99
LDFLAGS=-g -mmcu=$(MCU) -Wall -Os $(MAKE_IT_SMALL_FLAGS)
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude
AVRDUDE_OPTIONS=-p$(MCU) -cusbasp -Pusb
TARGET=rfm
OBJECTS=main.cpp.o uart.c.o RFM69.cpp.o SPI/SPI.cpp.o
HEADERS=uart.h constants.h
LIBRARIES=arduino/arduino.a

LFUSE?=0xEF
HFUSE?=0xDB
EFUSE?=0xFD

all: $(TARGET) $(TARGET).hex

%.c.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

%.cpp.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBRARIES) -o $(TARGET)

$(TARGET).hex: $(TARGET)
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET) $(TARGET).hex

upload: $(TARGET).hex
	$(AVRDUDE) -v $(AVRDUDE_OPTIONS) -Uflash:w:$(TARGET).hex:i

clean:
	rm $(OBJECTS) $(TARGET).hex $(TARGET)

readfuses: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_OPTIONS) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

writefuses: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_OPTIONS) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

.PHONY: upload clean readfuses writefuses
