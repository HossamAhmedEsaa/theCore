/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LIB_THREAD_FREERTOS_OS_UTILS_
#define LIB_THREAD_FREERTOS_OS_UTILS_

#include <cstdint>
#include <cstdint>

namespace ecl
{

//! OS-specific handle.
//! Yet we know that FreeRTOS uses void * as a typedef for TaskHandle_t
//! this will stay here.
using thread_handle = void *;

namespace this_thread
{

//!
//! \brief Yields current thread.
//! As for FreeRTOS whether current task will be rescheduled or not
//! depends on FreeRTOS configuration.
//!
void yield();

//!
//! \brief Put task at sleep for given amount of milliseconds.
//! \param[in] ms Milliseconds to sleep.
//!
void sleep_for(uint32_t ms);

//!
//! \brief Gets a thread handle of currently running thread.
//! \return Current thread handle
//!
thread_handle get_handle();

} // namespace this_thread

} // namespace ecl

#endif
