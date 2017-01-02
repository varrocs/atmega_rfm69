#MCU=attiny85
#AVRDUDEMCU=t85
MCU=atmega328p
CC=avr-gcc
CXX=avr-c++
CXXFLAGS=-g -Os -Wall -mcall-prologues -mmcu=$(MCU) -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS=-std=c99
OBJ2HEX=avr-objcopy
AVRDUDE=avrdude
AVRDUDE_OPTIONS=-p$(MCU) -cusbasp -Pusb
TARGET=rfm
OBJECTS=main.o
C_OBJECTS=uart.o
HEADERS=uart.h constants.h

LFUSE?=0xEF
HFUSE?=0xDB
EFUSE?=0xFD

all: $(TARGET) $(TARGET).hex

uart.o: uart.c
	$(CC) -c $(CFLAGS)  $(CXXFLAGS) $< -o $@

.o: %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJECTS) $(C_OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(C_OBJECTS) -o $(TARGET)

$(TARGET).hex: $(TARGET)
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET) $(TARGET).hex

upload: $(TARGET).hex
	$(AVRDUDE) -v $(AVRDUDE_OPTIONS) -Uflash:w:$(TARGET).hex:i

clean:
	rm $(OBJECTS) $(C_OBJECTS) $(TARGET).hex $(TARGET)

readfuses: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_OPTIONS) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

writefuses: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_OPTIONS) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

.PHONY: upload clean readfuses writefuses
