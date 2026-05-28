#include "nrf24l01.h"
#include "spi.h"
#include "RF24/nRF24L01.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

using namespace nRF24L01;

#define RF24_IRQ_ALL ( (1 << MASK_RX_DR) | (1 << MASK_TX_DS) | (1 << MASK_MAX_RT) )
#define RF24_RX_DR   (1 << RX_DR)
#define RF24_TX_DS   (1 << TX_DS)
#define RF24_MAX_RT  (1 << MAX_RT)

inline void RF24::beginTransaction() {
    PORTB &= ~(1 << SS);
}

inline void RF24::endTransaction() {
    PORTB |= (1 << SS);
}

void RF24::write_register(uint8_t reg, uint8_t value) {
    beginTransaction();
    status = Transfer_spi(W_REGISTER | (reg & REGISTER_MASK));
    Transfer_spi(value);
    endTransaction();
}

void RF24::write_register(uint8_t reg, const uint8_t* buf, uint8_t len) {
    beginTransaction();
    status = Transfer_spi(W_REGISTER | (reg & REGISTER_MASK));
    SendBuffer_spi(buf, len);
    endTransaction();
}

void RF24::read_register(uint8_t reg, uint8_t* buf, uint8_t len) {
    beginTransaction();
    status = Transfer_spi(R_REGISTER | (reg & REGISTER_MASK));
    while(len--) {
        *buf++ = Transfer_spi(0xFF);
    }
    endTransaction();
}

uint8_t RF24::read_register(uint8_t reg) {
    uint8_t result;
    beginTransaction();
    status = Transfer_spi(R_REGISTER | (reg & REGISTER_MASK));
    result = Transfer_spi(0xFF);
    endTransaction();
    return result;
}

RF24::RF24(uint32_t _spi_speed)
    : spi_speed(_spi_speed),
      payload_size(20),
      _is_p_variant(false),
      _is_p0_rx(false),
      ack_payloads_enabled(false),
      addr_width(5),
      dynamic_payloads_enabled(false)
{
    memset(pipe0_reading_address, 0, sizeof(pipe0_reading_address));
    memset(pipe0_writing_address, 0, sizeof(pipe0_writing_address));
    config_reg = 0;
}

bool RF24::begin(void) {

    DDRB |= (1 << CE) | (1 << SS);
    
    PORTB |= (1 << SS);
    PORTB &= ~(1 << CE);

    Init_spi();
    _delay_ms(5);

    write_register(SETUP_AW, 0x03);
    uint8_t check_val = read_register(SETUP_AW);

    if (check_val != 0x03) {
        return false; 
    }

    config_reg = 0x0C;
    write_register(CONFIG, config_reg);
    write_register(EN_AA, 0x3F);       
    write_register(EN_RXADDR, 0x03);   
    write_register(SETUP_RETR, 0x3F);
    write_register(RF_CH, 0x4C);       
    write_register(RF_SETUP, 0x0E);

    beginTransaction();
    Transfer_spi(FLUSH_RX);
    endTransaction();

    beginTransaction();
    Transfer_spi(FLUSH_TX);
    endTransaction();

    write_register(STATUS, RF24_IRQ_ALL);

    return true; 
}

bool RF24::isChipConnected() {
    return read_register(SETUP_AW) == 0x03;
}

void RF24::startListening(void) {
    config_reg |= (1 << PRIM_RX);
    write_register(CONFIG, config_reg);
    write_register(STATUS, RF24_IRQ_ALL); 
    
    PORTB |= (1 << CE);

    if (_is_p0_rx) {
        write_register(RX_ADDR_P0, pipe0_reading_address, addr_width);
    }
}

void RF24::stopListening(void) {
    PORTB &= ~(1 << CE);

    _delay_us(200); 

    if (ack_payloads_enabled) {
        beginTransaction();
        Transfer_spi(FLUSH_TX);
        endTransaction();
    }

    config_reg &= ~(1 << PRIM_RX);
    write_register(CONFIG, config_reg);
    
    write_register(EN_RXADDR, read_register(EN_RXADDR) | 1); 
}


bool RF24::available(void) {
    return (read_register(FIFO_STATUS) & (1 << RX_EMPTY)) == 0;
}

bool RF24::available(uint8_t* pipe_num) {
    if (available()) {
        if (pipe_num) {
            *pipe_num = (read_register(STATUS) >> RX_P_NO) & 0x07;
        }
        return true;
    }
    return false;
}

void RF24::read_payload(void* buf, uint8_t data_len) {
    uint8_t* current = reinterpret_cast<uint8_t*>(buf);
    uint8_t blank_len = 0;
    
    if (!dynamic_payloads_enabled) {
        data_len = (data_len < payload_size) ? data_len : payload_size;
        blank_len = payload_size - data_len;
    } else {
        data_len = (data_len < 32) ? data_len : 32;
    }

    beginTransaction();
    status = Transfer_spi(R_RX_PAYLOAD);
    while (data_len--) {
        *current++ = Transfer_spi(0xFF);
    }
    while (blank_len--) {
        Transfer_spi(0xFF);
    }
    endTransaction();
}

void RF24::read(void* buf, uint8_t len) {
    read_payload(buf, len);
    write_register(STATUS, RF24_RX_DR);
}

bool RF24::write(const void* buf, uint8_t len) {
    writeFast(buf, len);
    
    PORTB |= (1 << CE);
    _delay_us(20);
    PORTB &= ~(1 << CE);
    
    return txStandBy();
}

bool RF24::writeFast(const void* buf, uint8_t len) {
    uint8_t data_len  = (len < payload_size) ? len : payload_size;
    uint8_t blank_len = payload_size - data_len;

    beginTransaction();
    status = Transfer_spi(W_TX_PAYLOAD);
    const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);
    while (data_len--) {
        Transfer_spi(*current++);
    }
    while (blank_len--) {
        Transfer_spi(0x00);
    }
    endTransaction();
    return true;
}

void RF24::openWritingPipe(const uint8_t* address) {
    write_register(RX_ADDR_P0, address, addr_width);
    write_register(TX_ADDR, address, addr_width);
    
    memcpy(pipe0_writing_address, address, addr_width);
}



void RF24::openReadingPipe(uint8_t child, const uint8_t* address) {

    if (child == 0) {
        memcpy(pipe0_reading_address, address, addr_width);
        _is_p0_rx = true;
    }

    if (child <= 5) {

        uint8_t pipe_reg = RX_ADDR_P0 + child;

        if (child > 1) {
            write_register(pipe_reg, address, 1);
        } else {
            write_register(pipe_reg, address, addr_width);
        }

        write_register(RX_PW_P0 + child, payload_size);

        write_register(EN_RXADDR, read_register(EN_RXADDR) | (1 << child));
    }
}


void RF24::powerDown(void) {
    config_reg &= ~(1 << PWR_UP);
    write_register(CONFIG, config_reg);
}

void RF24::powerUp(void) {
    if (!(config_reg & (1 << PWR_UP))) {
        config_reg |= (1 << PWR_UP);
        write_register(CONFIG, config_reg);
        _delay_ms(5); 
    }
}

bool RF24::txStandBy() {

    while (!(read_register(FIFO_STATUS) & (1 << TX_EMPTY))) {

        uint8_t current_status = read_register(STATUS);

        if (current_status & RF24_MAX_RT) {

            write_register(STATUS, RF24_MAX_RT);

            beginTransaction();
            Transfer_spi(FLUSH_TX);
            endTransaction();

            return false;
        }
    }

    return true;
}
bool RF24::rxFifoFull() {
    return (read_register(FIFO_STATUS) & (1 << nRF24L01::FIFO_FULL)) != 0;
}


rf24_fifo_state_e RF24::isFifo(bool about_tx) {
    uint8_t state = (read_register(FIFO_STATUS) >> (4 * about_tx)) & 3;
    return static_cast<rf24_fifo_state_e>(state);
}

void RF24::ce(bool level) {
    if (level) {
        PORTB |= (1 << CE);
    } else {
        PORTB &= ~(1 << CE);
    }
}
