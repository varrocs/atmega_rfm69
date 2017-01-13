MCU=atmega328p
CC=avr-gcc
CXX=avr-c++
CXXFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MCU) -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS=-std=c99
LDFLAGS=-g -mmcu=$(MCU) -Wall
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude
AVRDUDE_OPTIONS=-p$(MCU) -cusbasp -Pusb
TARGET=rfm
OBJECTS=main.cpp.o uart.c.o
HEADERS=uart.h constants.h

LFUSE?=0xEF
HFUSE?=0xDB
EFUSE?=0xFD

all: $(TARGET) $(TARGET).hex

%.c.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(CXXFLAGS) $< -o $@

%.cpp.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

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
