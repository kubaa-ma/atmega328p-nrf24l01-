#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RF24_SPI_SPEED   10000000

typedef uint8_t rf24_gpio_pin_t;

enum rf24_fifo_state_e {
    FIFO_EMPTY = 0,
    FIFO_DATA,
    FIFO_FULL
};

class RF24 {
private:
    rf24_gpio_pin_t ce_pin;
    rf24_gpio_pin_t csn_pin;
    uint32_t spi_speed;

    uint8_t status;
    uint8_t payload_size;
    uint8_t pipe0_reading_address[5];
    uint8_t pipe0_writing_address[5];
    uint8_t config_reg;
    bool _is_p_variant;
    bool _is_p0_rx;

    inline void beginTransaction();
    inline void endTransaction();

    bool ack_payloads_enabled;
    uint8_t addr_width;
    bool dynamic_payloads_enabled;

    void write_register(uint8_t reg, const uint8_t* buf, uint8_t len);
    void read_register(uint8_t reg, uint8_t* buf, uint8_t len);
    uint8_t read_register(uint8_t reg);
    void read_payload(void* buf, uint8_t data_len);
    
public:
    void write_register(uint8_t reg, uint8_t value);
    RF24(uint32_t _spi_speed = RF24_SPI_SPEED);
    
    bool begin(void);   
    bool isChipConnected();
    void startListening(void);
    void stopListening(void);
    
    bool available(void);
    bool available(uint8_t* pipe_num);
    void read(void* buf, uint8_t len);
    bool write(const void* buf, uint8_t len);
    bool writeFast(const void* buf, uint8_t len);
    
    void openWritingPipe(const uint8_t* address);
    void openReadingPipe(uint8_t number, const uint8_t* address);
    
    void powerDown(void);
    void powerUp(void);
    bool txStandBy();
    
    bool rxFifoFull();
    rf24_fifo_state_e isFifo(bool about_tx);
    
    void ce(bool level);
};

