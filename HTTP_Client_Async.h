#pragma once
#ifndef HTTP_CLIENT_ASYNC
#define HTTP_CLIENT_ASYNC

// Example taken from: https://www.boost.org/doc/libs/develop/libs/beast/example/http/client/async/http_client_async.cpp
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, asynchronous
//
//------------------------------------------------------------------------------


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include "StdAfx.h"
#include "SysLatData.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
inline void
boostFail(beast::error_code ec, char const* what)
{
    //std::cerr << what << ": " << ec.message() << "\n";
    string error = what;
    error += ": " + ec.message() + "\n";
    DEBUG_PRINT(error)
}




// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    //http::request<http::empty_body> req_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit
        session(net::io_context& ioc)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc))
    {
    }

    // Start the asynchronous operation
    http::response<http::string_body>*    run(Json::Value dataToSend, char const* host, char const* port, char const* target, int version);
    void    on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void    on_write(beast::error_code ec, std::size_t bytes_transferred);
    void    on_read(beast::error_code ec, std::size_t bytes_transferred);

    inline auto get_response() { return res_;}
};


//char const* host, char const* port, char const* target, int version, char const* method, char const* contentType
inline http::response<http::string_body> upload_data(Json::Value dataToSend, char const* target = "/api/benchmarkData") {
    
    auto const host = "localhost";
    auto const port = "3000";
    int version = 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // Launch the asynchronous operation
    auto sharedPointer = std::make_shared<session>(ioc); //if you create the shared pointer on the RHS as I was before, then when it goes out of scope, the response will go with it.
    auto prv = sharedPointer->run(dataToSend, host, port, target, version);

    // Run the I/O service. The call will return when
    // the get operation is complete.
    
    ioc.run();
    http::response<http::string_body> rv = *prv;
    
    return rv;
}

#endif