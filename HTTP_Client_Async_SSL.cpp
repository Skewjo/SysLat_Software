#include "stdafx.h"
#include "HTTP_Client_Async_SSL.h"
#include <fstream>
#include<iostream>
#include <string>
#include <debugapi.h>
#include <wincrypt.h>
#pragma comment (lib, "crypt32")

void SSL_session::run(Json::Value dataToSend, char const* host, char const* port, char const* target, int version)
{
    if (!SSL_set_tlsext_host_name(stream_.native_handle(), host))
    {
        beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
        std::cerr << ec.message() << "\n";
        return;
    }

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
            &SSL_session::on_resolve,
            shared_from_this()));
}

void SSL_session::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return boostFail_secure(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream_).async_connect(
        results,
        beast::bind_front_handler(
            &SSL_session::on_connect,
            shared_from_this()));
}

void SSL_session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec)
        return boostFail_secure(ec, "connect");

    // Perform the SSL handshake
    stream_.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &SSL_session::on_handshake,
            shared_from_this()));
}

void SSL_session::on_handshake(beast::error_code ec)
{
    if (ec)
        return boostFail_secure(ec, "handshake");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream_, req_,
        beast::bind_front_handler(
            &SSL_session::on_write,
            shared_from_this()));
}

void SSL_session::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return boostFail_secure(ec, "write");

    // Receive the HTTP response
    http::async_read(stream_, buffer_, res_,
        beast::bind_front_handler(
            &SSL_session::on_read,
            shared_from_this()));
}



void SSL_session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return boostFail_secure(ec, "read");

    // Write the message to standard out - OutputDebugStringA for Windows...
    std::ostringstream debugOut;
    debugOut << res_ << std::endl;
    OutputDebugStringA(debugOut.str().c_str());


    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream
    stream_.async_shutdown(
        beast::bind_front_handler(
            &SSL_session::on_shutdown,
            shared_from_this()));
}

void SSL_session::on_shutdown(beast::error_code ec)
{
    if (ec == net::error::eof)
    {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if (ec)
        return boostFail_secure(ec, "shutdown");

    // If we get here then the connection is closed gracefully
}

