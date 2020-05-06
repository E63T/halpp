#pragma once
#include <halpp/detail.hpp>
#include <halpp/peripheral.hpp>

namespace hal
{
    class pin_base;

    class gpio_base
    {
    public:
        virtual uint8_t get_pin_count() = 0;
        virtual pin_base* get_pin(uint8_t index) = 0;
        virtual GPIO_TypeDef* const get_gpio() = 0;
    };

    using pin_mode = detail::pin_mode;

    class pin_base
    {
    public:
        virtual void mode(pin_mode) = 0;
        virtual void set() = 0;
        virtual void reset() = 0;
        virtual bool read() = 0;
        virtual void toggle() = 0;

        virtual gpio_base* parent() = 0;

        void write(bool value)
        {
            value ? set() : reset();
        }
    };

    template<char> class gpio;

    class pin : public pin_base
    {
    public:
        pin(gpio_base* parent, uint8_t const number) :
            m_parent(parent),
            m_number(number)
            {}

        pin() : m_number(0), m_parent(nullptr){}

        gpio_base* parent()
        {
            return m_parent;
        } 

        void mode(pin_mode m)
        {
            #ifdef STM32F1
                if(!m_parent) return;
                if(m == pin_mode::INPUT_PULLUP)
                {
                    m = pin_mode::INPUT_PULLDOWN;
                    m_parent->get_gpio()->ODR |= 1 << m_number;
                }
                else if(m == pin_mode::INPUT_PULLDOWN)
                {
                    m_parent->get_gpio()->ODR &= ~(1 << m_number);
                }

                register_t* reg = &m_parent->get_gpio()->CRL;
                uint8_t position = m_number & 7;
                if(m_number & 0x8)
                    reg = &m_parent->get_gpio()->CRH;

                *reg = (*reg & ~(0xF << position*4)) | (((uint8_t)m & 0xF) << position*4);
            #elif defined(__AVR_ARCH__)
                
            #endif
        }

        void set()
        {
            if(!m_parent) return;
            m_parent->get_gpio()->BSRR = 1 << m_number;
        }

        void reset()
        {
            if(!m_parent) return;
            m_parent->get_gpio()->BRR = 1 << m_number;
        }

        bool read()
        {
            if(!m_parent) return 0;
            return m_parent->get_gpio()->IDR & (1 << m_number);
        }

        void toggle()
        {
            if(!m_parent) return;
            m_parent->get_gpio()->ODR ^= (1 << m_number);
        }

    private:
        gpio_base* m_parent;
        uint8_t m_number;
        
        template<char> friend class gpio;
    };

    template<char PortName>
    class gpio : 
        public peripheral,
        public gpio_base
    {
    public:

        uint8_t get_pin_count()
        {
            return 16;
        }

        pin_base* get_pin(uint8_t index)
        {
            if(index < get_pin_count())
                return m_pins + index;
            else
                return nullptr;
        }

        gpio() : 
            peripheral(&RCC->APB2ENR, detail::GPIO_RCC_EMASK<PortName>),
            m_gpio(reinterpret_cast<GPIO_TypeDef*>(detail::GPIO_ADDR<PortName>))
        {
            for(uint8_t i = 0; i < 16; i++)
                m_pins[i] = pin(this, i);
        }

        GPIO_TypeDef* const get_gpio()
        {
            return m_gpio;
        }

    private:
        GPIO_TypeDef* const m_gpio;
        pin m_pins[16];  
    };

    template<char PortName>
    gpio<PortName> port;
}
