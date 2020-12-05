#include "stdafx.h"
#include "HTTP_Client_Async.h"
#include <fstream>
#include<iostream>
#include <string>
#include <debugapi.h>


void session::run(Json::Value dataToSend, char const* host, char const* port, char const* target, int version)
{
    // Set up an HTTP POST request message
    req_.version(version);
    req_.method(http::verb::post);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(beast::http::field::content_type, "application/json");
    
    Json::FastWriter fastWriter;
    std::string output = fastWriter.write(dataToSend);
    req_.body() = output;

    std::ostringstream debugOut;
    debugOut << req_ << std::endl;
    OutputDebugStringA(debugOut.str().c_str());
    req_.prepare_payload();

    // Look up the domain name
    resolver_.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &session::on_resolve,
            shared_from_this()));
}

void session::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return boostFail(ec, "resolve");

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    stream_.async_connect(
        results,
        beast::bind_front_handler(
            &session::on_connect,
            shared_from_this()));
}

void session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec)
        return boostFail(ec, "connect");

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream_, req_,
        beast::bind_front_handler(
            &session::on_write,
            shared_from_this()));
}

void session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return boostFail(ec, "write");

    // Receive the HTTP response
    http::async_read(stream_, buffer_, res_,
        beast::bind_front_handler(
            &session::on_read,
            shared_from_this()));
}



void session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return boostFail(ec, "read");

    // Write the message to standard out - OutputDebugStringA for Windows...
    //std::cout << res_ << std::endl;
    std::ostringstream debugOut;
    debugOut << res_ << std::endl;
    OutputDebugStringA(debugOut.str().c_str());


    // Gracefully close the socket
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected)
        return boostFail(ec, "shutdown");

    // If we get here then the connection is closed gracefully
}

