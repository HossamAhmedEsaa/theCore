# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Linker flags.
# TODO: check redundancy when using -msoft-float and -mfloat-abi=soft together

set(STM32_DEVICE "${thecore_cfg.platform.device}")

if(STM32_DEVICE MATCHES "STM32F4")
    set(FAMILY_CORE "-mcpu=cortex-m4")
elseif(STM32_DEVICE MATCHES "STM32L1")
    set(FAMILY_CORE "-mcpu=cortex-m3")
else()
    # Likely that additional implementation is required in order to add new stm32
    # family
    msg_fatal("Device ${CONFIG_PLATFORM_DEVICE} doesn't have any linker flags defined")
endif()

# TODO: alter FPU flags based on device used

set(CMAKE_C_LINK_FLAGS
    "-fuse-linker-plugin ${FAMILY_CORE} -nostartfiles -nostdlib \
        -mthumb -flto -Wl,--gc-sections"
	CACHE STRING "Linker C flags")
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} CACHE STRING "Linker C++ flags")

unset(STM32_DEVICE)
