# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(__thread_api_def)
    return()
endif()
set(__thread_api_def YES)

#-------------------------------------------------------------------------------
# theCore thread API

# Other OS does not yet have any API.
if(NOT CONFIG_OS STREQUAL "freertos")
    return()
endif()

# Sets directory where FreeRTOS configuration header resides.
# Syntax:
# theCore_freertos_header_config_dir(config_dir_path)
#   - config_dir_path - path to dir where configuration header is placed.
function(theCore_freertos_header_config_dir CONFIG_DIR_PATH)
    set(CONFIG_FREERTOS_HEADERS_PATH "${CONFIG_DIR_PATH}"
        CACHE PATH "FreeRTOS config location" FORCE)
endfunction()
