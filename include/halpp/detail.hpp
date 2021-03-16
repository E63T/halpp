#pragma once
#include <halpp/system.hpp>
#include <stdint.h>

#ifdef __AVR_ARCH__
    #include <avr/interrupt.h>
    #include <avr/io.h>
#endif

namespace hal
{ 
    namespace detail
    {
    template<typename T1, typename T2>
    inline constexpr uint32_t offset_of(T1 T2::*member)
    {
        constexpr T2 object{};
        return size_t(&(object.*member)) - size_t(&object);
    }

    template<typename ...>
    using void_t = void;

    #if defined(STM32F0) || defined(STM32F1) || defined(STM32F4)
        constexpr static const uint8_t PINS_PER_PORT = 16;
    #endif
    
    #if defined(STM32F0) || defined(STM32F4)
    
        #if defined(STM32F0)
            static register_t* const GPIO_ENR = &RCC->AHBENR;
            constexpr static uint32_t GPIO_RCC_EBB = RCC_AHBENR_GPIOAEN;
        #elif defined(STM32F4)
            static register_t* const GPIO_ENR = &RCC->AHB1ENR;
            constexpr static uint32_t GPIO_RCC_EBB = RCC_AHB1ENR_GPIOAEN;
        #endif
         template<char PortName>
        constexpr uint32_t GPIO_RCC_EMASK = GPIO_RCC_EBB << (PortName - 'A');

        template<char PortName>
        constexpr uint32_t GPIO_ADDR = GPIOA_BASE + 0x400 * (PortName - 'A');

        enum class pin_mode
        {
            //Name          =   MMTSSPP
            INPUT           = 0b0000000,
            INPUT_PULLUP    = 0b0000001,
            INPUT_PULLDOWN  = 0b0000010,
            OUTPUT_10M      = 0b0100000,
            OUTPUT          = 0b0100100,
            OUTPUT_50M      = 0b0101100,
            OUTPUT_OD_10M   = 0b0110000,
            OUTPUT_OD       = 0b0110100,
            OUTPUT_OD_50M   = 0b0111100,
            AFO_10M         = 0b1000000,
            AFO             = 0b1000100,
            AFO_50M         = 0b1001100,
            AFO_OD_10M      = 0b1010000,
            AFO_OD          = 0b1010100,
            AFO_OD_50M      = 0b1011100,
            ANALOG          = 0b1100000
        };

        using GPIO_TypeDef = ::GPIO_TypeDef;

        #if defined(STM32F0)
            extern "C"
            {
                void EXTI0_1_IRQHandler();

                void EXTI2_3_IRQHandler();

                void EXTI4_15_IRQHandler();
            }
        #elif defined(STM32F4)
            extern "C"
            {
                void EXTI0_IRQHandler();

                void EXTI1_IRQHandler();

                void EXTI2_IRQHandler();

                void EXTI3_IRQHandler();

                void EXTI4_IRQHandler();

                void EXTI9_5_IRQHandler();

                void EXTI15_10_IRQHandler();
            }
        #endif
        constexpr static const uint8_t EXTI_COUNT = 16; // TODO
    
    #elif defined(STM32F1)
        static register_t* GPIO_ENR = &RCC->APB2ENR;

        constexpr static uint32_t GPIO_RCC_EBB = RCC_APB2ENR_IOPAEN;

        template<char PortName>
        constexpr uint32_t GPIO_RCC_EMASK = GPIO_RCC_EBB << (PortName - 'A');

        template<char PortName>
        constexpr uint32_t GPIO_ADDR = GPIOA_BASE + 0x400 * (PortName - 'A');

        

        enum class pin_mode
        {
            INPUT_ANALOG = 0x0,
            OUTPUT_10M= 0x1,
            OUTPUT = 0x2,
            OUTPUT_50M = 0x3,
            INPUT = 0x4,
            OUTPUT_OD_10M= 0x5,
            OUTPUT_OD = 0x6,
            OUTPUT_OD_50M = 0x7,
            INPUT_PULLDOWN = 0x8,
            AFO_10M = 0x9,
            AFO = 0xA,
            AFO_50M = 0xB,
            INPUT_PULLUP = 0xC,
            AFO_OD_10M = 0xD,
            AFO_OD = 0xE,
            AFO_OD_50M = 0xF
        };

        class pin_base;

        using GPIO_TypeDef = ::GPIO_TypeDef;

        

        extern "C"
        {
            void EXTI0_IRQHandler();

            void EXTI1_IRQHandler();

            void EXTI2_IRQHandler();

            void EXTI3_IRQHandler();

            void EXTI4_IRQHandler();

            void EXTI9_5_IRQHandler();

            void EXTI15_10_IRQHandler();
        }

        constexpr static const uint8_t EXTI_COUNT = 16;
    #elif defined(__AVR_ARCH__)
        // TODO : define for other AVR MCUs

        struct GPIO_TypeDef
        {
            register_t* PORT;
            register_t* DDR;
            register_t* PIN;
        };

        template<char, typename T = void_t<> >
        GPIO_TypeDef port = (typename T::nonexistent_gpio_port)();

        #ifdef DDRA
            #define HALPP_PORT_NAME A
            #include <halpp/avr/gpio_typedef_template.h>
        #endif

        #ifdef DDRB
            #define HALPP_PORT_NAME B
            #include <halpp/avr/gpio_typedef_template.h>
        #endif

        #ifdef DDRC
            #define HALPP_PORT_NAME C
            #include <halpp/avr/gpio_typedef_template.h>
        #endif

        #ifdef DDRD
            #define HALPP_PORT_NAME D
            #include <halpp/avr/gpio_typedef_template.h>
        #endif

        enum class pin_mode
        {
            INPUT,
            INPUT_PULLUP,
            OUTPUT
        };

        constexpr static const uint8_t PINS_PER_PORT = 8;

        constexpr static const uint8_t EXTI_COUNT = 23; // INT0, INT1 + EXTI
    #endif
    }
}