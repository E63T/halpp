#pragma once
#include <halpp/function.hpp>
#include <halpp/detail.hpp>

namespace hal
{
    constexpr const static uint8_t FALLING = 1, RISING = 2;

    namespace interrupt    
    {
        bool register_exti(uint8_t line, uint8_t trig, hal::function<void()>&& handler, char port_name = 'A', bool force = false, uint8_t priority = 0);
        void unregister_exti(uint8_t line);
        bool is_free_exti(uint8_t line);
        void trigger_exti(uint8_t line);
    };
}