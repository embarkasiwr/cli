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

#include "cli/cli.h"
#include "cli/remotecli.h"
#include "cli/server.h"

using namespace cli;
using namespace std;

/////////////


class AsyncCli
{
public:
    AsyncCli( boost::asio::io_service& ios, CliSession& _session ) :
        session( _session ),
        input( ios, ::dup( STDIN_FILENO ) )
    {
        session.Add( "exit", [this](std::ostream&){ session.Exit(); }, "Quit the application" );
        Read();
    }
    ~AsyncCli()
    {
        input.close();
    }

private:

    void Read()
    {
        session.Prompt();
        // Read a line of input entered by the user.
        boost::asio::async_read_until(
            input,
            inputBuffer,
            '\n',
            std::bind( &AsyncCli::NewLine, this,
                       std::placeholders::_1,
                       std::placeholders::_2 )
        );
    }

    void NewLine( const boost::system::error_code& error, std::size_t length )
    {
        if ( !error || error == boost::asio::error::not_found )
        {
            auto bufs = inputBuffer.data();
            std::size_t size = length;
            if ( !error ) --size; // tolgo il \n
            std::string s( boost::asio::buffers_begin( bufs ), boost::asio::buffers_begin( bufs ) + size );
            inputBuffer.consume( length );


            if ( session.Feed( s ) ) Read();
        }
        else
        {
            input.close();
        }
    }

    CliSession& session;
    boost::asio::streambuf inputBuffer;
    boost::asio::posix::stream_descriptor input;
};

/////////////

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
            "answer",
            [](int x, std::ostream& out){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything " );

    auto subMenu = make_unique< Menu >( "sub" );
    subMenu -> Add(
            "hello",
            [](std::ostream& out){ out << "Hello, submenu world\n"; },
            "Print hello world in the submenu" );
    rootMenu -> Add( std::move(subMenu) );


    Cli cli( std::move(rootMenu) );
    // global exit action
    cli.ExitAction( [](auto& out){ out << "Goodbye and thanks for all the fish.\n"; } );

    CliSession session( cli, std::cout );
    session.ExitAction( [&ios](auto& out) // session exit action
            {
                out << "Closing App...\n";
                ios.stop();
            } );
    AsyncCli ac( ios, session );

    // setup server

    CliServer server( ios, 5000, cli );
    // exit action for all the connections
    server.ExitAction( [](auto& out) { out << "Terminating this session...\n"; } );
    ios.run();

    return 0;
}
