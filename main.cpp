#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "nrf24l01.h"
#include "RF24/nRF24L01.h"

#define LED PB0
#define PAYLOAD_SIZE 20

RF24 radio;

const uint8_t AVR_ADDR[5] = {'A', 'V', 'R', '_', '1'};
const uint8_t ARD_ADDR[5] = {'A', 'R', 'D', '_', '1'};

int main(void) {

    DDRB |= (1 << LED);
    PORTB &= ~(1 << LED);

    uint8_t input[PAYLOAD_SIZE];
    uint8_t output[PAYLOAD_SIZE];

    radio.begin();

    radio.powerUp();

    radio.write_register(nRF24L01::RF_CH, 76);
    radio.write_register(nRF24L01::RF_SETUP, 0x0E);
    radio.write_register(nRF24L01::EN_AA, 0x3F);

    radio.openWritingPipe(ARD_ADDR);
    radio.openReadingPipe(1, AVR_ADDR);

    radio.startListening();

    while (1) {

        if (radio.available()) {

            memset(input, 0, PAYLOAD_SIZE);

            radio.read(input, PAYLOAD_SIZE);

            radio.stopListening();
            _delay_us(150);
            memset(output, 0, PAYLOAD_SIZE);

            switch(input[0]) {

                case '0':

                    PORTB &= ~(1 << LED);

                    strcpy((char*)output, "LED OFF");

                    break;

                case '1':

                    PORTB |= (1 << LED);

                    strcpy((char*)output, "LED ON");

                    break;

                default:

                    strcpy((char*)output, "UNKNOWN");

                    break;
            }

            radio.write(output, PAYLOAD_SIZE);

            radio.startListening();
            _delay_us(150);
        }

        _delay_ms(1);
    }

    return 0;
}