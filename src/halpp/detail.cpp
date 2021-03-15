#include <halpp/detail.hpp>

#if defined(STM32F1) || defined(STM32F0)
#elif defined(__AVR_ARCH__)

#else
    #error Unsupported MCU
#endif