#include "spi.h"

#include <stdint.h>
#include <avr/io.h>

void Init_spi(){
    DDRB |= (1 << SS) | (1 << SCK) | (1 << MOSI);
        // Defining Outputs (MISO is input)
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
//   |              |           |               |
//   |              |           |               |
//   |              |           |               x===== (Speed of SPI)
//   |              |           x==== (Master Mode)
//   |              x====== (Switchning ON SPI)
//   x==== (Spi control Registr)
}

uint8_t Transfer_spi(uint8_t data){
    SPDR = data;
        //  SPDR – full duplex register

    while(!(SPSR &(1 << SPIF)));
//      Waiting untill status registr SPIF flag will be true (means that data was send)
    return SPDR;

}

void SendBuffer_spi(const uint8_t *data, uint16_t length){
    for(uint16_t i = 0; i < length; i++){
        SPDR = *data++;

        while(!(SPSR &(1 << SPIF)));
    }
}