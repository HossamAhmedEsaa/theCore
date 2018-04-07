/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "application.h"
#include "stdarg.h"

#include "aux/platform_defines.hpp"

//------------------------------------------------------------------------------
// System timer

//! User-overridable timer event handler
extern "C"
__attribute__((weak))
void systmr_handler()
{
    // Do nothing if not overriden
}

//! Struct-container for timer callback and corresponding object.
static struct tmr_cb
{
    //! Handles timeout events
    void on_timeout()
    {
        event_cnt++;
        systmr_handler();
    }

    //! Total timer events counter. May wrap.
    volatile uint32_t event_cnt = 0;
} cb_obj;

//! Timer speed in hertz.
static constexpr auto tmr_speed_hz = THECORE_SYSTMR_FREQ;

//! Particle timer object, used as system timer.
static Timer particle_tmr{1000 / tmr_speed_hz, &tmr_cb::on_timeout, cb_obj};

namespace ecl
{

namespace systmr
{

 //! Enables timer, start counting
void enable()
{
    particle_tmr.start();
}

//! Disables timer, stop counting
void disable()
{
    particle_tmr.stop();
}

uint32_t speed()
{
    return tmr_speed_hz;
}

uint32_t events()
{
    return cb_obj.event_cnt;
}

} // namespace systmr


} // namespace ecl
