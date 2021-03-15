#pragma once

namespace hal
{
    namespace clock
    {
        static external_source lse(32768);
        static external_source hse(8000000);
        static source hsi(8000000);
        static source lsi(40000);


        enum class hse_prediv_value
        {
            DIV1,
            DIV2,
            MAX
        };

        static divisor<hse_prediv_value> hse_prediv(hse, &RCC->CFGR, 17);
        static static_divisor<2> hsi_2(hsi);

        enum class pll_src_value
        {
            HSI2,
            HSE,
            MAX
        };

        static node_base* pll_srcs[] = {&hsi_2, &hse};

        static mux<pll_src_value> pll_src_mux(&RCC->CFGR, 16, pll_srcs);

        static multiplier<uint8_t, 17> pll_mul(pll_src_mux, &RCC->CFGR, 18);

        enum class sysclock_mux_value
        {
            HSI,
            HSE,
            PLL,
            MAX
        };

        static node_base* sw_srcs[] = {&hsi, &hse, &pll_mul};

        static mux<sysclock_mux_value> sysclock_mux(&RCC->CFGR, 0, sw_srcs);

        static static_divisor<1> usb_prescaler(pll_mul);

        enum class ahb_prescaler_value
        {
            DIV1,
            DIV2 = 8,
            DIV4, 
            DIV8,
            DIV16,
            DIV64,
            DIV128,
            DIV256,
            DIV512,
            MAX
        };

        template<>
        constexpr const uint8_t get_numeric_value<ahb_prescaler_value>(ahb_prescaler_value v)
        {
            switch(v)
            {
                case ahb_prescaler_value::DIV2:
                    return 1;
                case ahb_prescaler_value::DIV4:
                    return 2;
                case ahb_prescaler_value::DIV8:
                    return 3;
                case ahb_prescaler_value::DIV16:
                    return 4;
                case ahb_prescaler_value::DIV64:
                    return 6;
                case ahb_prescaler_value::DIV128:
                    return 7;
                case ahb_prescaler_value::DIV256:
                    return 8;
                case ahb_prescaler_value::DIV512:
                    return 9;
                default:
                    return 0;
            }
        }

        static divisor<ahb_prescaler_value> ahb_prescaler(sysclock_mux, &RCC->CFGR, 3);

        enum class apb_prescaler_value
        {
            DIV1,
            DIV2 = 4,
            DIV4,
            DIV8,
            DIV16,
            MAX
        };

        template<>
        constexpr const uint8_t get_numeric_value<apb_prescaler_value>(apb_prescaler_value v)
        {
            if((int)v < 4)
                return 0;
            else
                return (int)v - 3;
        }

        static divisor<apb_prescaler_value> apb1_prescaler(ahb_prescaler, &RCC->CFGR, 8);
        static divisor<apb_prescaler_value> apb2_prescaler(ahb_prescaler, &RCC->CFGR, 8);

        static static_multiplier<2> apb1_timer(apb1_prescaler);
        static static_multiplier<2> apb2_timer(apb2_prescaler);
    }
}