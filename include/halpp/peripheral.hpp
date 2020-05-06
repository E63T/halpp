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
    
    class peripheral : public peripheral_base
    {
    public:
        peripheral(register_t* const reg, const uint32_t mask) : 
            m_enabled(false),
            m_register(reg),
            m_mask(mask){}

        void enable()
        {
            *m_register |= m_mask;
            m_enabled = true;
        }

        void disable()
        {
            *m_register &= ~m_mask;
            m_enabled = false;
        }

        bool enabled()
        {
            return m_enabled;
        }
    private:
        bool m_enabled;
        register_t *const m_register;
        uint32_t const m_mask;
    };
}