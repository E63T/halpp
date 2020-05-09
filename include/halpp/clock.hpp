#pragma once
#include <halpp/system.hpp>
#include <type_traits>

namespace hal
{
    namespace clock
    {
        using frequency_t = uint32_t;

        class node_base
        {
        public:
            virtual frequency_t frequency() = 0;
        
        };

        class source : public node_base
        {
        public:
            source(frequency_t fr) : m_frequency(fr) {}

            frequency_t frequency()
            {
                return m_frequency;
            }

        protected:
            frequency_t m_frequency;    
        };

        class external_source : public source
        {
        public:
            external_source(frequency_t fr) : source(fr){};

            void assume_frequency(frequency_t fr)
            {
                m_frequency = fr;
            }
        };

        class node : public node_base
        {
        public:
            node(node_base& parent) : m_parent(parent){}
            
            node_base& parent()
            {
                return m_parent;
            }

        private:
            node_base& m_parent;
        };

        template<typename T, typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
        const static uint8_t bit_width(const T number)
        {
            constexpr uint8_t max_width = sizeof(T) * 8;

            for(unsigned i = max_width; i > 0; i--)
            {
                if (number & (1 << (i - 1)))
                    return i;
            }

            return 0;
        }

        template<typename T>
        constexpr static const uint8_t get_numeric_value(T t)
        {
            return (uint8_t)t;
        }



        template<typename T, T max = T::MAX>
        class reg
        {
        public:
            reg(register_t* r, uint8_t bit_pos) :
                m_reg(r),
                m_bit_pos(bit_pos)
                {}

            void set(T t)
            {
                uint8_t bits = bit_width((int)max - 1);
                uint8_t rev_mask = ~(((1 << bits) - 1) << m_bit_pos);
                *m_reg = (*m_reg & rev_mask) | ((int)t << m_bit_pos);
            }

            T get()
            {
                uint8_t bits = bit_width((int)max - 1);
                uint8_t mask = (1 << bits) - 1;
                return static_cast<T>((*m_reg >> m_bit_pos) & mask);
            }
        private:
            register_t* m_reg;
            uint8_t m_bit_pos;
        };

        template<typename T, T max = T::MAX>
        class divisor : public reg<T, max>, public node
        {
        public:
            divisor(node_base& parent, register_t* r, uint8_t bit_pos) :
                reg<T, max>(r, bit_pos),
                node(parent)
                {}

            frequency_t frequency()
            {
                return parent().frequency() >> get_numeric_value(reg<T, max>::get());
            }
                
        };

        template<uint32_t N>
        class static_divisor : public node
        {
        public:
            static_divisor(node_base& p) :
                node(p)
                {}

            frequency_t frequency()
            {
                return parent().frequency() / N;
            }
        };

        template<typename T, T max = T::MAX>
        class multiplier : public reg<T, max>, public node
        {
        public:
            multiplier(node_base& parent, register_t* r, uint8_t bit_pos) :
                reg<T, max>(r, bit_pos),
                node(parent)
                {}

            frequency_t frequency()
            {
                return parent().frequency() * get_numeric_value(reg<T,max>::get());
            }
                
        };

        template<uint32_t N>
        class static_multiplier : public node
        {
        public:
            static_multiplier(node_base& p) :
                node(p)
                {}

            frequency_t frequency()
            {
                return parent().frequency() * N;
            }
        };

        template<typename T, T max = T::MAX>
        class mux : public reg<T, max>, public node_base
        {
        public:
            mux(register_t* r, uint8_t bit_pos, node_base** parents) :
                reg<T, max>(r, bit_pos)
            {
                for(uint32_t i = 0; i < (uint32_t)max; i++)
                {
                    m_parents[i] = parents[i];
                }
            }

            frequency_t frequency()
            {
                return m_parents[get_numeric_value(reg<T,max>::get())]->frequency();
            }

        private:
            node_base* m_parents[(uint32_t) max];
        };

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

        // TODO
    }
}