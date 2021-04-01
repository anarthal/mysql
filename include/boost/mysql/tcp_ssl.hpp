//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TCP_SSL_HPP
#define BOOST_MYSQL_TCP_SSL_HPP

#include <boost/mysql/socket_connection.hpp>
#include <boost/mysql/resultset.hpp>
#include <boost/mysql/prepared_statement.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace boost {
namespace mysql {

/// A connection to MySQL over a TCP socket using TLS.
using tcp_ssl_connection = socket_connection<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

/// A prepared statement associated to a [reflink tcp_ssl_connection].
using tcp_ssl_prepared_statement = prepared_statement<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

/// A resultset associated with a [reflink tcp_ssl_connection].
using tcp_ssl_resultset = resultset<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

} // mysql
} // boost

#endif