#include <halpp/16x2.hpp>
#include <halpp/systick.hpp>
#include <initializer_list>
#include <cstring>

hal::lcd16x2::lcd16x2(pin_base &rs, pin_base &rw, pin_base &e, pin_base &d4, pin_base &d5, pin_base &d6, pin_base &d7) :
    m_rs(rs),
    m_rw(rw),
    m_e(e),
    m_d4(d4),
    m_d5(d5),
    m_d6(d6),
    m_d7(d7)
    {}

void hal::lcd16x2::send4(uint8_t data)
{
    this->m_d7.write(data & 0x8);
    this->m_d6.write(data & 0x4);
    this->m_d5.write(data & 0x2);
    this->m_d4.write(data & 0x1);
    strobe();
}

void hal::lcd16x2::strobe()
{
    this->m_e.set();
    hal::systick::delay(1);
    this->m_e.reset();
    hal::systick::delay(1);
}

void hal::lcd16x2::send8(uint8_t data)
{
    send4(data >> 4);
    send4(data);
}

void hal::lcd16x2::command(uint8_t cmd)
{
    this->m_rs.reset();
    send8(cmd);
}

void hal::lcd16x2::data(uint8_t data)
{
    this->m_rs.set();
    send8(data);
}

void hal::lcd16x2::init()
{
    this->m_rw.mode(hal::pin_mode::OUTPUT_10M);
    this->m_rs.mode(hal::pin_mode::OUTPUT_10M);
    this->m_e.mode(hal::pin_mode::OUTPUT_10M);
    this->m_d4.mode(hal::pin_mode::OUTPUT_10M);
    this->m_d5.mode(hal::pin_mode::OUTPUT_10M);
    this->m_d6.mode(hal::pin_mode::OUTPUT_10M);
    this->m_d7.mode(hal::pin_mode::OUTPUT_10M);
    this->m_rw.reset();
    hal::systick::delay(50);
    this->m_e.reset();
    this->m_rs.reset();
    send4(0x2);
    hal::systick::delay(2);
    send4(0x2);
    hal::systick::delay(2);
    send4(0x2);
    hal::systick::delay(2);
    send4(0x2);
    send4(0x2);
    command(0xC);
    command(0x28);
    command(0x06);
    command(0x0C);
    command(0);
    command(0x28);
    command(1);
    command(6);
    command(0x0C);
    clear();
}

void hal::lcd16x2::clear()
{
    command(1);
}

void hal::lcd16x2::set_address(uint8_t addr)
{
    command(addr | 0x80);
}

void hal::lcd16x2::set_address(uint8_t x, uint8_t y)
{
    uint8_t addr = x + (y * 0x40);
    set_address(addr);
}

void hal::lcd16x2::wait_busy()
{
    this->m_rw.set();
    this->m_d7.mode(hal::pin_mode::INPUT);
    do 
    {
        strobe();
    } 
    while(m_d7.read());
    this->m_rw.reset();
    this->m_d7.mode(hal::pin_mode::OUTPUT_10M);
}

void hal::lcd16x2::puts(char* d)
{
    while(*d)
    {
        data(*d);
        d++;
    }
}
