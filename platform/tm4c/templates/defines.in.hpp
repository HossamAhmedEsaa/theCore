/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//! \file
//! \brief TM4C platform definitions template

#ifndef TM4C_PLATFORM_DEFINES_
#define TM4C_PLATFORM_DEFINES_

/*[[[cog
import cog
import json

f = open(JSON_CFG)
cfg = json.load(f)
cfg = cfg['menu-platform']['menu-tm4c']

]]]*/
//[[[end]]]

// Console configuration -------------------------------------------------------
/*[[[cog

if 'config-console' in cfg:
    cog.outl('#define THECORE_CONFIG_USE_CONSOLE 1')

]]]*/
//[[[end]]]
// Timer configuration ---------------------------------------------------------
#if THECORE_NO_OS
    /*[[[cog

    if 'menu-timer' in cfg:
        # Systick as a systimer is only possibility for timer selection for this
        # platform. So for now, let's skip check for 'owner' field.
        systmr_cfg = cfg['menu-timer']
        assert(systmr_cfg['config-source'] == 'systick')

        freq_hz = 50
        if 'config-freq' in systmr_cfg:
            freq_hz = systmr_cfg['config-freq']

        cog.outl('#define THECORE_ENABLE_SYSTMR_API 1')
        cog.outl('#define THECORE_CONFIG_SYSTMR_FREQ %d' % int(freq_hz))

        if systmr_cfg['config-owner'] == 'thecore':
            cog.outl('#define THECORE_OWNS_SYSTMR 1')

    ]]]*/
    //[[[end]]]
#endif

// WFI/WFE routines presense
#define THECORE_USE_WFI_WFE 1
// IRQ manager is not needed, interrupts are registered in compile-time
#define THECORE_NO_IRQ_MANAGER 1


#endif // TM4C_PLATFORM_DEFINES_
