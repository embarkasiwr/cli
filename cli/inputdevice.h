/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef INPUTDEVICE_H_
#define INPUTDEVICE_H_

#include <functional>
#include <string>
#include <boost/asio.hpp>

namespace cli
{

enum class KeyType { ascii, up, down, left, right, backspace, canc, home, end, ret, ignored };

class InputDevice
{
public:
    using Handler = std::function< void( std::pair<KeyType,char> ) >;

    explicit InputDevice(boost::asio::io_service& ios) : ioService(ios) {}
    virtual ~InputDevice() = default;

    template <typename H>
    void Register(H&& h) { handler = std::forward<H>(h); }

protected:

    void Notify(std::pair<KeyType,char> k)
    {
        ioService.post( [this,k](){ if (handler) handler(k); } );
    }

private:

    boost::asio::io_service& ioService;
    Handler handler;
};

} // namespace cli

#endif // INPUTDEVICE_H_

