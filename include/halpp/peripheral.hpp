#pragma once
#include <halpp/system.hpp>

namespace hal
{
    class peripheral_base
    {
    public:
        virtual void enable() = 0;
        virtual void disable() = 0;
        virtual bool enabled() = 0;
    };

    class dummy_peripheral
    {
    public:
        void enable(){}
        void disable(){}
        bool enabled(){ return true; }
    };
    
    class peripheral : public peripheral_base
    {
    public:
        peripheral(register_t* const reg, const uint32_t mask);

        void enable();

        void disable();

        bool enabled();
    private:
        bool m_enabled;
        register_t *const m_register;
        uint32_t const m_mask;
    };

    #ifdef __AVR_ARCH__
        #define HALPP_DUMMY_GPIO_PERIPHERAL
        using gpio_peripheral = dummy_peripheral;
    #else
        using gpio_peripheral = peripheral;
    #endif

    #if defined(STM32F0) || defined(STM32F1) || defined(STM32F4)
        #define HALPP_AFIO_PRESENT
        extern peripheral afio;
    #endif
}