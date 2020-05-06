#pragma once

#ifdef STM32F1
    #include <stm32f1xx.h>
#elif defined(__AVR_ARCH__)
    #include <avr/io.h>
#else
    #error This MCU is not supported
#endif

namespace hal
{
    extern uint32_t FREQUENCY;

    #if defined(STM32F1)
        using register_t = __IO uint32_t;
    #elif defined(__AVR_ARCH__)
        using register_t = volatile uint8_t;
    #endif


    #ifndef HALPP_DISABLE_LIBC_INIT_ARRAY
        extern "C" void __libc_init_array(void);
    #endif

    extern void init();
    
}