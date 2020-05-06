#include <halpp/system.hpp>
#include <halpp/systick.hpp>

namespace hal::systick
{
    volatile uint32_t systick_ctr = 0;

    void init()
    {
        SysTick->LOAD = FREQUENCY / 1000 - 1;
        SysTick->VAL = 0;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
    }



    void SysTick_Handler()
    {
        systick_ctr++;
    }

    void delay(uint32_t ms)
    {
        uint32_t old_stctr = systick_ctr;
        while(systick_ctr < old_stctr + ms);
    }


}