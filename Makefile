CC = avr-g++
MCU = atmega328p
PROGRAMMER = usbasp
F_CPU = 16000000UL

CFLAGS = -Wall -Os -std=c++11 -DF_CPU=$(F_CPU) -mmcu=$(MCU)

OBJCOPY = avr-objcopy

SRC = main.cpp spi.cpp nrf24l01.cpp

INCLUDE = -I. -IRF24

TARGET = main.elf
HEX = main.hex

all: $(HEX)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(TARGET)

$(HEX): $(TARGET)
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET) $(HEX)

flash: $(HEX)
	avrdude -c $(PROGRAMMER) -p m328p -U flash:w:$(HEX):i

clean:
	rm -f $(TARGET) $(HEX)
