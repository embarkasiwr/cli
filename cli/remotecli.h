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

#ifndef REMOTECLI_H_
#define REMOTECLI_H_

#include <memory>
#include "cli/cli.h"
#include "cli/server.h"

namespace cli
{

class TcpCliSession : public Session
{
public:
    TcpCliSession( boost::asio::ip::tcp::socket socket, Cli& _cli, std::function< void(std::ostream&)> exitAction ) :
        Session( std::move( socket ) ),
        cliSession( _cli, this -> OutStream() )
    {
        cliSession.ExitAction( exitAction );
        cliSession.Add( std::make_unique< FuncCmd >( "exit", [this](std::ostream&){ cliSession.Exit(); }, "Terminate this session" ) );
    }

protected:

    virtual void OnConnect() override
    {
        cliSession.Prompt();
    }
    virtual void OnDisconnect() override {}
    virtual void OnError() override {}
    virtual void OnDataReceived( const std::string& data ) override
    {
        auto str = data;
        // trim trailing spaces
        std::size_t endpos = str.find_last_not_of(" \t\r\n");
        if( std::string::npos != endpos ) str = str.substr( 0, endpos+1 );

        if ( cliSession.Feed( str ) ) cliSession.Prompt();
        else Disconnect();
    }
private:
    CliSession cliSession;
};

class CliServer : public Server
{
public:
    CliServer( boost::asio::io_service& ios, short port, Cli& _cli ) :
        Server( ios, port ),
        cli( _cli )
    {}
    void ExitAction( std::function< void(std::ostream&)> action )
    {
        exitAction = action;
    }
    virtual std::shared_ptr< Session > CreateSession( boost::asio::ip::tcp::socket socket ) override
    {
        return std::make_shared< TcpCliSession >( std::move( socket ), cli, exitAction );
    }
private:
    Cli& cli;
    std::function< void(std::ostream&)> exitAction;
};

} // namespace

#endif // REMOTECLI_H_

