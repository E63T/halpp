#pragma once

#ifdef STM32F1
    #include <stm32f1xx.h>
#elif defined(STM32F030x6) || defined(STM32F030x4)
    #include <stm32f030x6.h>
#elif defined(STM32F042x6)
    #include <stm32f042x6.h>
#elif defined(STM32F401xE)
    #include <stm32f401xe.h>
#elif defined(STM32F405)
    #include <stm32f405xx.h>
#elif defined(STM32F407)
    #include <stm32f407xx.h>
#elif defined(STM32F429)
    #include <stm32f429xx.h>
#elif defined(__AVR_ARCH__)
    #include <avr/io.h>
#else
    #error This MCU is not supported
#endif


#include <cstddef>

namespace hal
{
    extern uint32_t FREQUENCY;

    #if defined(STM32F1) || defined(STM32F0) || defined(STM32F4)
        using register_t = __IO uint32_t;
    #elif defined(__AVR_ARCH__)
        using register_t = volatile uint8_t;
    #endif


    #ifndef HALPP_DISABLE_LIBC_INIT_ARRAY
        extern "C" void __libc_init_array(void);
    #endif

    extern void init();
    
}