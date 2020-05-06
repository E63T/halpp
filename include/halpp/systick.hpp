#pragma once
#include <halpp/system.hpp>

// TODO: Use programmable frequency

namespace hal
{

    namespace systick
    {
        extern volatile uint32_t systick_ctr;

        extern "C" void SysTick_Handler();

        extern void init();

        extern void delay(uint32_t ms);
    }


}