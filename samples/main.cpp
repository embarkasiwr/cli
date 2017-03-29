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

#include "cli/server.h" 
// TODO. NB: server.h includes boost asio, so in Windows it should compare before cli.h
// and inputhandler.h that include rang
// (consider to provide a global header file for the library)
#include "cli/cli.h"
#include <cli/inputhandler.h>
#include "cli/remotecli.h"
#include "cli/keyboard.h"
#include "cli/clilocalsession.h"

using namespace cli;
using namespace std;


int main()
{
    boost::asio::io_service ios;

    // setup cli

    auto rootMenu = make_unique< Menu >( "cli" );
    rootMenu -> Add(
            "hello",
            [](std::ostream& out){ out << "Hello, world\n"; },
            "Print hello world" );
    rootMenu -> Add(
            "hello_everysession",
            [](std::ostream&){ Cli::cout() << "Hello, everybody" << std::endl; },
            "Print hello everybody on all open sessions" );
    rootMenu -> Add(
            "answer",
            [](int x, std::ostream& out){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything " );
    rootMenu -> Add(
            "color",
            [](std::ostream& out){ out << "Colors ON\n"; SetColor(); },
            "Enable colors in the cli" );
    rootMenu -> Add(
            "nocolor",
            [](std::ostream& out){ out << "Colors OFF\n"; SetNoColor(); },
            "Disable colors in the cli" );

    auto subMenu = make_unique< Menu >( "sub" );
    subMenu -> Add(
            "hello",
            [](std::ostream& out){ out << "Hello, submenu world\n"; },
            "Print hello world in the submenu" );
    subMenu -> Add(
            "demo",
            [](std::ostream& out){ out << "This is a sample!\n"; },
            "Print a demo string" );

    auto subSubMenu = make_unique< Menu >( "subsub" );
        subSubMenu -> Add(
            "hello",
            [](std::ostream& out){ out << "Hello, subsubmenu world\n"; },
            "Print hello world in the sub-submenu" );
    subMenu -> Add( std::move(subSubMenu));

    rootMenu -> Add( std::move(subMenu) );


    Cli cli( std::move(rootMenu) );
    // global exit action
    cli.ExitAction( [](auto& out){ out << "Goodbye and thanks for all the fish.\n"; } );

    // TODO: incorporate CliSession inside CliLocalSession
    CliSession session( cli, std::cout );
    session.ExitAction( [&ios](auto& out) // session exit action
            {
                out << "Closing App...\n";
                ios.stop();
            } );
    CliLocalSession ls(ios, session);

    // setup server

    CliTelnetServer server(ios, 5000, cli);
    // exit action for all the connections
    server.ExitAction( [](auto& out) { out << "Terminating this session...\n"; } );
    ios.run();

    return 0;
}
