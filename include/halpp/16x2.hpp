#pragma once
#include <halpp/gpio.hpp>

namespace hal
{
    class lcd16x2
    {
    public:
        lcd16x2(pin_base &rs, pin_base &rw, pin_base &e, pin_base &d4, pin_base &d5, pin_base &d6, pin_base &d7);
        void init();
        void clear();
        void set_cursor(bool cursor, bool blink);
        void data(uint8_t);
        void command(uint8_t);
        void set_address(uint8_t x, uint8_t y);
        void set_address(uint8_t addr);
        void wait_busy();
        void puts(char* data);
    private:
        pin_base &m_rs, &m_rw, &m_e, &m_d4, &m_d5, &m_d6, &m_d7;

        void send4(uint8_t);
        void send8(uint8_t);
        void strobe();
    };
}