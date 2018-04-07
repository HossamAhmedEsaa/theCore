/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


//! \addtogroup platform Platform defintions and drivers
//! @{

//! \addtogroup tm4c Texas Instruments Tiva C TM4C123G platform
//! @{

//! \defgroup tm4c_console Console
//! @{

//! \file
//! \brief TM4C bypass console implementation.

#include "platform/console.hpp"

#include <type_traits>

namespace ecl
{

void bypass_console_init()
{
    constexpr auto uart_dev = ecl::extract_value(ecl::console_channel);

    SysCtlPeripheralEnable(platform_console::pick_sysctl());

    UARTConfigSetExpClk(uart_dev, SysCtlClockGet(), 115200,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
}

void bypass_putc(char c)
{
    constexpr auto uart_dev = ecl::extract_value(ecl::console_channel);
    UARTCharPut(uart_dev, c);
}

} // namespace ecl

//! @}

//! @}

//! @}
