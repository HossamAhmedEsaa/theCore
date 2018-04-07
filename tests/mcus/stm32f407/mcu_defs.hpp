/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//! \file
//! \brief stm32f407 exported definitions and peripherals, required by tests

#ifndef TEST_STM32F407_MCU_DEFS_HPP_
#define TEST_STM32F407_MCU_DEFS_HPP_

#include <aux/spi_bus.hpp>
#include <aux/i2c_bus.hpp>
#include <platform/gpio_device.hpp>

namespace ecl
{

namespace test
{

// Available SPIs

using spi1 = spi_i2s_bus<spi_device::bus1>;
using spi_i2s2 = spi_i2s_bus<spi_device::bus2>;
using spi_i2s3 = spi_i2s_bus<spi_device::bus3>;

// Available GPIOs

using gpio_pa0 = gpio<gpio_port::a, gpio_num::pin0>;


} // namespace test

} // namespace ecl


#endif // TEST_STM32F407_MCU_DEFS_HPP_
