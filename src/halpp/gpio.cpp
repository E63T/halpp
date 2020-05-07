#include <halpp/gpio.hpp>
#include <halpp/interrupt.hpp>

void hal::pin_base::write(bool v)
{
    v ? set() : reset();
}

hal::pin::pin(gpio_base* parent, uint8_t const number):
    m_parent(parent),
    m_number(number)
    {}

hal::pin::pin() : m_number(0), m_parent(nullptr){}

void hal::pin::mode(pin_mode m)
{
    if(!m_parent) return;
    #ifdef STM32F1
        
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
        register_t* ddr = m_parent->get_gpio()->DDR;
        register_t* port = m_parent->get_gpio()->PORT;
        switch(m)
        {
        case INPUT_PULLUP:
            *port |= _BV(m_number);
            *ddr &= ~_BV(m_number);
            break;
        case INPUT:
            *port &= ~_BV(m_number);
            *ddr &= ~_BV(m_number);
            break;
        case OUTPUT:
            *ddr |= ~_BV(m_number);
            break;
        }
    #else
        #error Unsupported MCU
    #endif
}

void hal::pin::set()
{
    if(!m_parent) return;
    #if defined(STM32F1)
        m_parent->get_gpio()->BSRR = 1 << m_number;
    #elif defined(__AVR_ARCH__)
        *(m_parent->get_gpio()->PORT) |= _BV(m_number);
    #else
        #error Unsupported MCU
    #endif
}

void hal::pin::reset()
{
    if(!m_parent) return;

    #if defined(STM32F1)
        m_parent->get_gpio()->BRR = 1 << m_number;
    #elif defined(__AVR_ARCH__)
        *(m_parent->get_gpio()->PORT) &= ~_BV(m_number);
    #else
        #error Unsupported MCU
    #endif
}

bool hal::pin::read()
{
    if(!m_parent) return 0;
    
    #if defined(STM32F1)
        return m_parent->get_gpio()->IDR & (1 << m_number);
    #elif defined(__AVR_ARCH__)
        return *(m_parent->get_gpio()->PIN) & _BV(m_number);
    #else
        #error Unsupported MCU
    #endif
}

bool hal::pin::read_output()
{
    if(!m_parent) return 0;
    
    #if defined(STM32F1)
        return m_parent->get_gpio()->ODR & (1 << m_number);
    #elif defined(__AVR_ARCH__)
        return *(m_parent->get_gpio()->PORT) & _BV(m_number);
    #else
        #error Unsupported MCU
    #endif
}

void hal::pin::toggle()
{
    if(!m_parent) return;
    
    #if defined(STM32F1)
        m_parent->get_gpio()->ODR ^= (1 << m_number);
    #elif defined(__AVR_ARCH__)
        *(m_parent->get_gpio()->PORT) &= _BV(m_number);
    #else
        #error Unsupported MCU
    #endif
}


uint8_t hal::pin::get_exti_line()
{
    #ifdef STM32F1
        return m_number;
    #elif defined(__AVR_ARCH__)
        if(!m_parent) return 255;

        return (m_parent->get_name() - 'B') * 8 + m_number;
    #endif
}

bool hal::pin::can_register_exti()
{
    if(!m_parent) return false;

    return hal::interrupt::is_free_exti(get_exti_line());
}

bool hal::pin::interrupt(uint8_t trig,hal::function<void()>&& h, bool force)
{
    if(!m_parent) return false;

    return hal::interrupt::register_exti(
        get_exti_line(),
        trig,
        std::forward<hal::function<void()> >(h),
        m_parent->get_name(),
        force
    );
}

