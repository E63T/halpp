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
        static source hsi(8000000);
        static source lsi(40000);

        static divisor<uint8_t, 17> hse_prediv(hse, &RCC->CFGR2, 0);
        static divisor<uint8_t, 17> hsi_prediv(hsi, &RCC->CFGR2, 0);
        static static_divisor<2> hsi_2(hsi);

        // TODO: HSI48
        
        static node_base* pll_srcs[] = {&hsi_2, &hsi_prediv, &hse_prediv};
        
        enum class pll_src_value
        {
            HSI2,
            HSI_PREDIV,
            HSE_PREDIV,
            MAX
        };

        static mux<pll_src_value> pll_src_mux(&RCC->CFGR, 15, pll_srcs);

        static multiplier<uint8_t, 17> pll_mul(pll_src_mux, &RCC->CFGR, 18);

        static node_base* sysclock_srcs[] = {&hsi, &hse, &pll_mul};

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