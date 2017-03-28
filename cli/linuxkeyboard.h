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

#ifndef LINUXKEYBOARD_H_
#define LINUXKEYBOARD_H_

#include <thread>
#include <memory>
#include <atomic>
#include <boost/asio.hpp>

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "inputdevice.h"


namespace cli
{

class LinuxKeyboard : public InputDevice
{
public:
    explicit LinuxKeyboard(boost::asio::io_service& ios) :
        InputDevice(ios)
    {
        ToManualMode();
        servant = std::make_unique<std::thread>( [this](){ Read(); } );
        servant -> detach();
    }
    ~LinuxKeyboard()
    {
        run = false;
        ToStandardMode();
    }

private:

    void Read()
    {
        while ( run )
        {
            auto k = Get();
            Notify(k);
        }
    }

    std::pair<KeyType,char> Get()
    {
        while ( !KbHit() ) {}
        int ch = getchar();
        switch( ch )
        {
            case 127: return std::make_pair(KeyType::backspace,' '); break;
            case 10: return std::make_pair(KeyType::ret,' '); break;
            case 27: // symbol
                ch = getchar();
                if ( ch == 91 ) // arrow keys
                {
                    ch = getchar();
                    switch( ch )
                    {
                        case 51:
                            ch = getchar();
                            if ( ch == 126 ) return std::make_pair(KeyType::canc,' ');
                            else return std::make_pair(KeyType::ignored,' ');
                            break;
                        case 65: return std::make_pair(KeyType::up,' '); break;
                        case 66: return std::make_pair(KeyType::down,' '); break;
                        case 68: return std::make_pair(KeyType::left,' '); break;
                        case 67: return std::make_pair(KeyType::right,' '); break;
                        case 70: return std::make_pair(KeyType::end,' '); break;
                        case 72: return std::make_pair(KeyType::home,' '); break;
                    }
                }
                break;
            default: // ascii
            {
                const char c = static_cast<char>(ch);
                return std::make_pair(KeyType::ascii,c);
            }
        }
        return std::make_pair(KeyType::ignored,' ');
    }

    void ToManualMode()
    {
        tcgetattr( STDIN_FILENO, &oldt );
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    }
    void ToStandardMode()
    {
        tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    }

    static int KbHit()
    {
      struct timeval tv;
      fd_set rdfs;

      tv.tv_sec = 1;
      tv.tv_usec = 0;

      FD_ZERO(&rdfs);
      FD_SET (STDIN_FILENO, &rdfs);

      select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
      return FD_ISSET(STDIN_FILENO, &rdfs);
    }

    termios oldt;
    termios newt;
    std::atomic<bool> run{ true };
    std::unique_ptr<std::thread> servant;
};

} // namespace

#endif // LINUXKEYBOARD_H_

