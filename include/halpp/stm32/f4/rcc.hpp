#pragma once
#include <halpp/clock.hpp>

namespace hal
{
    namespace clock
    {
        class hse_node : public external_source, public reg<uint8_t, 2>
        {
        public:
            hse_node(size_t freq, register_t* r, uint8_t bp) :
                external_source(freq),
                reg<uint8_t, 2>(r, bp)
                {}

            void enable()
            {
                reg::set(1);
            }

            void disable()
            {
                reg::set(0);
            }

            void assume_frequency(size_t f)
            {
                #ifdef HALPP_HSE_AUTOENABLE
                    enable();
                #endif

                external_source::assume_frequency(f);
            }
        };

        static external_source lse(32768);
        static hse_node hse(8000000, &RCC->CR, RCC_CR_HSEON_Pos);
        static source hsi(16000000);
        static source lsi(40000);
        
        static node_base* pll_srcs[] = {&hsi, &hse};
        
        enum class pll_src_value
        {
            HSI,
            HSE,
            MAX
        };

        static mux<pll_src_value> pll_src_mux(&RCC->PLLCFGR, 22, pll_srcs);

        static divisor<uint8_t, 64> pll_m(pll_src_mux, &RCC->PLLCFGR, 0);
        static multiplier<uint16_t, 512> pll_n(pll_m, &RCC->PLLCFGR, 6);
        
        enum class pll_p_value
        {
            DIV2,
            DIV4,
            DIV6,
            DIV8,
            MAX
        };

        static divisor<pll_p_value> pll_p(pll_n, &RCC->PLLCFGR, 16);


        static node_base* sysclock_srcs[] = {&hsi, &hse, &pll_p};

        enum class sysclock_src_value
        {
            HSI,
            HSE,
            PLL,
            MAX
        };

        static mux<sysclock_src_value> sysclock_mux(&RCC->CFGR, 0, sysclock_srcs);


        // TODO


    }
}