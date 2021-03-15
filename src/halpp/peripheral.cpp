#include <halpp/peripheral.hpp>

hal::peripheral::peripheral(hal::register_t* const reg, const uint32_t mask)  : 
    m_enabled(false),
    m_register(reg),
    m_mask(mask){};

void hal::peripheral::enable()
{
    *m_register |= m_mask;
    m_enabled = true;
}

void hal::peripheral::disable()
{
    *m_register &= ~m_mask;
    m_enabled = false;
}

bool hal::peripheral::enabled()
{
    return m_enabled;
}

#if defined(STM32F1)
    hal::peripheral hal::afio(&RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
#endif
