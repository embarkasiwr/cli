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

#ifndef POLLKEYBOARDINPUT_H_
#define POLLKEYBOARDINPUT_H_

#include <functional>
#include <string>
#include "terminal.h"
#include "keyboard.h"
#include "cli.h" // CliSession

// forward declaraction
namespace boost { namespace asio { class io_service; } }

namespace cli
{

class PollKeyboardInput
{
public:
    PollKeyboardInput(boost::asio::io_service& ios, CliSession& _session) :
        session( _session ),
        terminal( ios, [this](auto cmd){ this->NewCommand(cmd); } )
    {
        session.Add( "exit", [this](std::ostream&){ session.Exit(); }, "Quit the application" );
        session.Prompt();
    }

private:

    void NewCommand( std::pair< Symbol, std::string > s )
    {
        switch ( s.first )
        {
            case Symbol::command:
                if ( session.Feed( s.second ) )
                    session.Prompt();
                break;
            case Symbol::down:
                terminal.SetLine( session.NextCmd() );
                break;
            case Symbol::up:
                terminal.SetLine( session.PreviousCmd() );
                break;
            case Symbol::tab:
                break;
        }

    }

    CliSession& session;
    Terminal< Keyboard > terminal;
};

} // namespace

#endif // POLLKEYBOARDINPUT_H_

