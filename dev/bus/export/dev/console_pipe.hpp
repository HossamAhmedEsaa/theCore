/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEV_BUS_CONSOLE_PIPE_
#define DEV_BUS_CONSOLE_PIPE_

//!
//! \file
//! \brief      Console pipe module.
//! \copyright
//! \todo Description and examples
//!

#include <ecl/err.hpp>
#include <unistd.h>

namespace ecl
{

//!
//! \brief Console pipe (blocking) adapter.
//! This adapter provides functionality of a blocking pipe
//! built over generic bus interface. It echoes each byte received.
//! Temporary solution, until 'serial' driver will be supported in each platform.
//! \tparam GBus Generic bus driver.
//! \sa generic_bus
//!
template< class GBus >
class console_pipe
{
public:
    //!
    //! \brief Constructs a pipe.
    //!
    console_pipe();
    //!
    //! \brief Destructs a pipe.
    //!
    ~console_pipe();

    //!
    //! \brief Inits a pipe.
    //! Lazy initialization. Inits a bus.
    //! \return Status of operation.
    //!
    ecl::err init();

    //!
    //! \brief Writes a data to a pipe.
    //! If error is occurred before transfer has started then this method
    //! will immediately exit and return negative integer value to notify
    //! about error.
    //! If error occur during transaction then return value will be less than
    //! bytes requested or zero, indicating amount of bytes that were
    //! successfully written to a pipe.
    //! Last error can be obtained via \sa last_error() call
    //! Caller will be blocked until all bytes will be transferred
    //! or error will occur.
    //! \pre Bus is initialized.
    //! \param[in]  data     Data to write. Must not be null.
    //! \param[in]  count    Count of bytes that must be written to a pipe.
    //!                      Can be zero.
    //! \return     Value indicating either error if negative or
    //!             amount of bytes transferred if positive.
    ssize_t write(const uint8_t *data, size_t count);

    //!
    //! \brief Reads a data from a pipe.
    //! If error is occurred before transfer has started then this method
    //! will immediately exit and return negative integer value to notify
    //! about error.
    //! If error occur during transaction or there is less data than request
    //! then return value will be less than size of provided buffer,
    //! indicating amount of bytes that were successfully read from a pipe.
    //! Last error can be obtained via \sa last_error() call.
    //! Caller will be blocked until all bytes will be written to a buffer
    //! or error will occur.
    //! \pre Bus is initialized and buffer is valid.
    //! \param[out] buffer     Data buffer to read to. Must not be null.
    //! \param[in]  size      Size of a buffer. Can be zero.
    //! \return     Value indicating an error if negative or bytes stored
    //!             to a buffer if positive.
    //!
    ssize_t read(uint8_t *buffer, size_t size);

    //!
    //! \brief Gets last error occurred.
    //! \return Error defined in ecl::err
    //!
    ecl::err last_error() const;

private:
    err  m_last;    //!< Last bus error.

    //!
    //! \brief Checks if error occur before xfer has started
    //! \retval true Error occurs on start
    //! \retval false Error hasn't occur at all, or occurred during xfer
    //!
    bool err_on_start() const;
};


template< class GBus >
console_pipe< GBus >::console_pipe()
    :m_last{err::ok}
{

}

template< class GBus >
console_pipe< GBus >::~console_pipe()
{

}

template< class GBus >
ecl::err console_pipe< GBus >::init()
{
    return (m_last = GBus::init());
}

template< class GBus >
ssize_t console_pipe< GBus >::write(const uint8_t *data, size_t count)
{
    ecl_assert(data);

    if (!count)
        return count;

    size_t sent = 0;
    ssize_t ret;

    GBus::lock();
    GBus::set_buffers(data, nullptr, count);

    m_last = GBus::xfer(&sent);

    GBus::unlock();

    ret = sent;

    if (err_on_start()) {
        // Error occur, right at the start
        ret = -1;
    }

    return ret;
}

template< class GBus >
ssize_t console_pipe< GBus >::read(uint8_t *buffer, size_t size)
{
    ecl_assert(buffer);

    if (!size)
        return size;

    size_t read = 0;
    ssize_t ret;

    GBus::lock();
    GBus::set_buffers(nullptr, buffer, size);

    m_last = GBus::xfer(nullptr, &read);

    GBus::unlock();

    ret = read;

    if (err_on_start()) {
        // Error occur, right at the start
        ret = -1;
    }

    // Echo all symbols back
    write(buffer, read);

    return ret;
}

template< class GBus >
err console_pipe< GBus >::last_error() const
{
    return m_last;
}

//------------------------------------------------------------------------------

template< class GBus >
bool console_pipe< GBus >::err_on_start() const
{
    return is_error(m_last) && !(m_last == err::io);
}

} // namespace ecl

#endif // DEV_BUS_CONSOLE_PIPE_
