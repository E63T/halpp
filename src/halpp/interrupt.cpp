#include <halpp/interrupt.hpp>
#include <halpp/peripheral.hpp>
#include <halpp/function.hpp>

static volatile hal::function<void()> exti_handlers[hal::detail::EXTI_COUNT] = {nullptr};


#ifdef __AVR_ARCH__
    #include <avr/io.h>
    #include <avr/interrupt.h>

    static uint8_t exti_trig[hal::detail::EXTI_COUNT] = {0};

    constexpr static bool is_pcint(uint8_t line)
    {
        #ifdef HALPP_AVR_USE_INTS
            return line != 19 && line != 20; // For Atmega328P : use INT0/1 instead of PCINT19/20
        #else
            return true;
        #endif
    }
#endif

bool hal::interrupt::is_free_exti(uint8_t line)
{
    return line < hal::detail::EXTI_COUNT && !((bool)exti_handlers[line]);
}

void hal::interrupt::trigger_exti(uint8_t line)
{
    if(!hal::interrupt::is_free_exti(line))
        exti_handlers[line]();
}

volatile bool hal::interrupt::register_exti(uint8_t line, uint8_t trig, hal::function<void()>&& handler, char port_name, bool force, uint8_t prio)
{
    if(line >= hal::detail::EXTI_COUNT)
        return false;

    if(!force && !hal::interrupt::is_free_exti(line)) 
        return false;

    exti_handlers[line] = std::move(handler);

    #if defined(STM32F1) || defined(STM32F0) || defined(STM32F4)
        if(!trig) return false;

        #if defined(STM32F1)
            hal::afio.enable();

        #endif

        uint8_t port_idx = port_name - 'A';

        uint8_t reg_idx = line / 4;
        uint8_t bit_pos = (line & 3) * 4;

        #if defined(STM32F0) || defined(STM32F4)
            SYSCFG->EXTICR[reg_idx] = (SYSCFG->EXTICR[reg_idx] & ~(0xF << bit_pos)) | (port_idx << bit_pos); 
        #elif defined(STM32F1)

            AFIO->EXTICR[reg_idx] = (AFIO->EXTICR[reg_idx] & ~(0xF << bit_pos)) | (port_idx << bit_pos); 
        #endif

        if(trig & hal::FALLING)
            EXTI->FTSR |= 1 << line;
        else
            EXTI->FTSR &= ~(1 << line);

        if(trig & hal::RISING)
            EXTI->RTSR |= 1 << line;
        else
            EXTI->RTSR &= ~(1 << line);

        EXTI->PR |= 1 << line;
        EXTI->IMR |= 1 << line;


        

        #if defined(STM32F1) || defined(STM32F4)
            decltype(EXTI0_IRQn) irq;

            if(line == 0)
                irq = EXTI0_IRQn;
            else if(line == 1)
                irq = EXTI1_IRQn;
            else if(line == 2)
                irq = EXTI2_IRQn;
            else if(line == 3)
                irq = EXTI3_IRQn;
            else if(line == 4)
                irq = EXTI4_IRQn;
            else if(line <= 9)
                irq = EXTI9_5_IRQn;
            else if(line <= 15)
                irq = EXTI15_10_IRQn;
        #elif defined(STM32F0)
            decltype(EXTI0_1_IRQn) irq;
            switch(line)
            {
                case 0:
                case 1:
                    irq = EXTI0_1_IRQn;
                    break;
                case 2:
                case 3:
                    irq = EXTI2_3_IRQn;
                    break;
                default:
                    irq = EXTI4_15_IRQn;
            }
        #endif

        NVIC_EnableIRQ(irq);
    #elif defined(__AVR_ARCH__)
        exti_trig[line] = trig;

        (void)port_name;
        (void)prio;

        if(is_pcint(line))
        {
            register_t* pcmsk = &PCMSK0;
            auto reg_idx = line >> 3;
            PCICR |= _BV(reg_idx);
            pcmsk[reg_idx] |= _BV(line & 7);
        }
        else
        {
            auto int_num = line - 19;

            bool falling = trig & FALLING;
            bool rising = trig & RISING;

            uint8_t isc = 0;

            if(falling && rising)
                isc = 1;
            else if(falling);
                isc = 2;
            else if(rising)
                isc = 3;

            EICRA = (EICRA & ~(0x3 << (int_num * 2))) | (isc << (int_num*2));
            EIMSK |= _BV(int_num);
        }
    #endif

    return true;
}

void hal::interrupt::unregister_exti(uint8_t line)
{
    #if defined(STM32F1)
        // TODO
    #elif defined(__AVR_ARCH__)
        if(is_pcint(line))
        {
            register_t* pcmsk = &PCMSK0;
            auto reg_idx = line >> 3;
            
            pcmsk[reg_idx] &= ~_BV(line & 7);
            
            if(pcmsk[reg_idx] == 0)
                PCICR &= ~_BV(reg_idx);
        }
        else
        {
            auto int_num = line - 19;
            EIMSK &= ~_BV(int_num);
        }
    #endif
}

#if defined(STM32F0)

extern "C"
{
    void EXTI0_1_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
        for(uint8_t idx = 0; idx <= 1; idx++)
        {
            if(EXTI->PR & (1 << idx))
            {
                EXTI->PR |= (1 << idx);
                hal::interrupt::trigger_exti(idx);
            }
        }
    }

    void EXTI2_3_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
        for(uint8_t idx = 2; idx <= 3; idx++)
        {
            if(EXTI->PR & (1 << idx))
            {
                EXTI->PR |= (1 << idx);
                hal::interrupt::trigger_exti(idx);
            }
        }
    }

    void EXTI4_15_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
        for(uint8_t idx = 4; idx <= 15; idx++)
        {
            if(EXTI->PR & (1 << idx))
            {
                EXTI->PR |= (1 << idx);
                hal::interrupt::trigger_exti(idx);
            }
        }
    }
}

#elif defined(STM32F1) || defined(STM32F4)

    extern "C" void EXTI0_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI0_IRQn);
        EXTI->PR |= 1 << 0;
        hal::interrupt::trigger_exti(0);
    }

    extern "C" void EXTI1_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI1_IRQn);
        EXTI->PR |= 1 << 1;
        hal::interrupt::trigger_exti(1);
    }

    extern "C" void EXTI2_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI2_IRQn);
        EXTI->PR |= 1 << 2;
        hal::interrupt::trigger_exti(2);
    }

    extern "C" void EXTI3_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI3_IRQn);
        EXTI->PR |= 1 << 3;
        hal::interrupt::trigger_exti(3);
    }

    extern "C" void EXTI4_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI4_IRQn);
        EXTI->PR |= 1 << 4;
        hal::interrupt::trigger_exti(4);
    }

    extern "C" void EXTI9_5_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        for(uint8_t idx = 5; idx <= 9; idx++)
        {
            if(EXTI->PR & (1 << idx))
            {
                EXTI->PR |= (1 << idx);
                hal::interrupt::trigger_exti(idx);
            }
        }
    }

    extern "C" void EXTI15_10_IRQHandler()
    {
        NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
        for(uint8_t idx = 10; idx <= 15; idx++)
        {
            if(EXTI->PR & (1 << idx))
            {
                EXTI->PR |= (1 << idx);
                hal::interrupt::trigger_exti(idx);
            }
        }
    }
#elif defined(__AVR_ARCH__)
    #ifdef HALPP_AVR_USE_INTS
        ISR(INT0_vect)
        {
            hal::interrupt::trigger_exti(19);
        }

        ISR(INT1_vect)
        {
            hal::interrupt::trigger_exti(20);
        }
    #endif

    static volatile uint8_t saved_pcint_state[3] = {0};

    static volatile void pcint_handler(register_t* port, uint8_t idx)
    {
        uint8_t new_state = *port; // TODO : Define ports for pcint
        uint8_t changed = new_state ^ saved_pcint_state[idx];
        for(uint8_t i = 0; i < 8; i++)
        {
            if(!is_pcint(idx * 8 + i)) continue;
            if(changed & _BV(i))
            {
                if(new_state & _BV(i))
                {
                    if(exti_trig[idx * 8 + i] & RISING)
                        trigger_exti(idx * 8 + i);
                }
                else
                {
                    if(exti_trig[idx * 8 + i] & FALLING)
                        trigger_exti(idx * 8 + i);
                }
            }
        }

        saved_pcint_state[idx] = new_state;
    }

    ISR(PCINT0_vect)
    {
        pcint_handler(&PORTB, 0);
    }

    ISR(PCINT1_vect)
    {
        pcint_handler(&PORTC, 1);
    }

    ISR(PCINT2_vect)
    {
        pcint_handler(&PORTD, 2);
    }

#endif