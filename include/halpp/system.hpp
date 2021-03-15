#pragma once


#ifdef STM32F1
    #include <stm32f1xx.h>
#elif defined(STM32F030x6) || defined(STM32F030x4)
    #include <stm32f030x6.h>
#elif defined(STM32F042x6)
    #include <stm32f042x6.h>
#elif defined(STM32F0) || defined(STM32F030) ||defined(STM32F042)
    #error Please specify more concrete MCU (e.g. STM32F030x6)
#elif defined(__AVR_ARCH__)
    #include <avr/io.h>
#else
    #error This MCU is not supported
#endif

#include <cstddef>

namespace hal
{
    extern uint32_t FREQUENCY;

    #if defined(STM32F1) || defined(STM32F0)
        using register_t = __IO uint32_t;
    #elif defined(__AVR_ARCH__)
        using register_t = volatile uint8_t;
    #endif


    #ifndef HALPP_DISABLE_LIBC_INIT_ARRAY
        extern "C" void __libc_init_array(void);
    #endif

    extern void init();
    
}