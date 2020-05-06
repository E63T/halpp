#include <halpp/system.hpp>
#include <halpp/systick.hpp>

namespace hal
{
    uint32_t FREQUENCY = 56000000;

    void init()
    {
        #ifndef HALPP_DISABLE_LIBC_INIT_ARRAY
            __libc_init_array();
        #endif

        systick::init();
    }
}
