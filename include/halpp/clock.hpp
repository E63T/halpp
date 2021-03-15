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

        
    }
}

#if defined(STM32F1)
    #include <halpp/stm32/f1/rcc.hpp>
#elif defined(STM32F0)
    #include <halpp/stm32/f0/rcc.hpp>
#endif