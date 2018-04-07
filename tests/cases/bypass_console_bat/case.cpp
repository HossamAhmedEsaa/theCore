/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <common/execution.hpp>
#include <platform/console.hpp>

// Headers are included in the last place to avoid some
// overrides of stdlib functions (like calloc/malloc/etc.)
#include <unity.h>
#include <unity_fixture.h>

TEST_GROUP(bypass_console_bat);

TEST_SETUP(bypass_console_bat)
{
}

TEST_TEAR_DOWN(bypass_console_bat)
{
}

TEST(bypass_console_bat, print_ascii)
{
    ecl::bypass_putc('\r');
    ecl::bypass_putc('\n');

    for (char c = 0x20; c < 0x7f; ++c) {
        ecl::bypass_putc(c);
        if (((c - 0x20) & 0xf) == 0xf) {
            ecl::bypass_putc('\r');
            ecl::bypass_putc('\n');
        }
    }
}

