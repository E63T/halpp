#pragma once
#include <halpp/detail.hpp>
#include <halpp/peripheral.hpp>
#include <halpp/function.hpp>

namespace hal
{
    class pin_base;

    class gpio_base
    {
    public:
        virtual uint8_t get_pin_count() = 0;
        virtual pin_base* get_pin(uint8_t index) = 0;
        virtual detail::GPIO_TypeDef* const get_gpio() = 0;
        virtual char get_name() const = 0;
    };

    using pin_mode = detail::pin_mode;

    class pin_base
    {
    public:
        virtual void mode(pin_mode) = 0;
        virtual void set() = 0;
        virtual void reset() = 0;
        virtual bool read() = 0;
        virtual bool read_output() = 0;
        virtual void toggle() = 0;

        virtual gpio_base* parent() = 0;
        virtual bool can_register_exti() = 0;
        virtual bool interrupt(uint8_t, hal::function<void()>&&, bool = false) = 0;

        void write(bool value);
    };

    template<char> class gpio;

    class pin : public pin_base
    {
    public:
        pin(gpio_base* parent, uint8_t const number);

        pin();

        gpio_base* parent()
        {
            return m_parent;
        } 

        void mode(pin_mode m);

        void set();

        void reset();

        bool read();

        bool read_output();

        void toggle();

        bool can_register_exti();
        
        bool interrupt(uint8_t trig, hal::function<void()>&& handler, bool force = false);

        uint8_t get_exti_line();

    private:
        gpio_base* m_parent;
        uint8_t m_number;
        
        template<char> friend class gpio;
    };

    template<char PortName>
    class gpio : 
        public gpio_peripheral,
        public gpio_base
    {
    public:

        uint8_t get_pin_count()
        {
            return detail::PINS_PER_PORT;
        }

        pin_base* get_pin(uint8_t index)
        {
            if(index < get_pin_count())
                return m_pins + index;
            else
                return nullptr;
        }

        gpio() : 
            #if !defined(HALPP_DUMMY_GPIO_PERIPHERAL)
                peripheral(&RCC->APB2ENR, detail::GPIO_RCC_EMASK<PortName>),
            #endif
            m_gpio(reinterpret_cast<detail::GPIO_TypeDef*>(detail::GPIO_ADDR<PortName>))
        {
            for(uint8_t i = 0; i < 16; i++)
                m_pins[i] = pin(this, i);
        }

        detail::GPIO_TypeDef* const get_gpio()
        {
            return m_gpio;
        }

        virtual char get_name() const
        {
            return PortName;
        }
    private:
        detail::GPIO_TypeDef* const m_gpio;
        pin m_pins[detail::PINS_PER_PORT];  
    };

    template<char PortName>
    gpio<PortName> port;
}
