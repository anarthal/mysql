//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/mp11/list.hpp>
#include "tcp_future_socket.hpp"

#ifndef BOOST_MYSQL_TEST_INTEGRATION_STREAM_LIST_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_STREAM_LIST_HPP


namespace boost {
namespace mysql {
namespace test {

// Make stream names shorter
using tcp_socket = boost::asio::ip::tcp::socket;
using tcp_ssl_socket = boost::asio::ssl::stream<tcp_socket>;
using tcp_ssl_future_socket = boost::asio::ssl::stream<tcp_future_socket>;

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
using unix_socket = boost::asio::local::stream_protocol::socket;
using unix_ssl_socket = boost::asio::ssl::stream<unix_socket>;
#endif

// Stream lists
using all_non_ssl_streams = boost::mp11::mp_list<
    tcp_socket,
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    unix_socket,
#endif
    tcp_future_socket
>;

using all_ssl_streams = boost::mp11::mp_list<
    tcp_ssl_socket,
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    unix_ssl_socket,
#endif
    tcp_ssl_future_socket
>;

using all_streams = boost::mp11::mp_list<
    tcp_socket,
    tcp_ssl_socket,
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    unix_socket,
    unix_ssl_socket,
#endif
    tcp_future_socket,
    tcp_ssl_future_socket
>;

// Stream names
template <class Stream> constexpr const char* get_stream_name();
template <> constexpr const char* get_stream_name<tcp_socket>() { return "tcp"; }
template <> constexpr const char* get_stream_name<tcp_future_socket>() { return "tcp_default_token"; }
template <> constexpr const char* get_stream_name<tcp_ssl_socket>() { return "tcp_ssl"; }
template <> constexpr const char* get_stream_name<tcp_ssl_future_socket>() { return "tcp_ssl_default_token"; }

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
template <> constexpr const char* get_stream_name<unix_socket>() { return "unix"; }
template <> constexpr const char* get_stream_name<unix_ssl_socket>() { return "unix_ssl"; }
#endif

// Supports SSL (doesn't use the lib's type trait for test independance)
template <class Stream> constexpr bool supports_ssl();
template <> constexpr bool supports_ssl<tcp_socket>() { return false; }
template <> constexpr bool supports_ssl<tcp_future_socket>() { return false; }
template <> constexpr bool supports_ssl<tcp_ssl_socket>() { return true; }
template <> constexpr bool supports_ssl<tcp_ssl_future_socket>() { return true; }

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
template <> constexpr bool supports_ssl<unix_socket>() { return false; }
template <> constexpr bool supports_ssl<unix_ssl_socket>() { return true; }
#endif

} // test
} // mysql
} // boost

#endif