//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_plch19g_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_plch19g_HPP

#include "../network_variant.hpp"
#include <boost/mysql/socket_connection.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/io_context.hpp>
#include <type_traits>

namespace boost {
namespace mysql {
namespace test {

class tcp_variant : public network_variant
{
public:
    bool supports_ssl() const override { return false; }
    const char* stream_name() const override { return "tcp"; }
};

template <class Stream>
socket_connection<Stream> create_socket_connection_impl(
    boost::asio::io_context::executor_type executor,
    boost::asio::ssl::context& ssl_ctx,
    std::true_type // is ssl stream
)
{
    return socket_connection<Stream>(executor, ssl_ctx);
}

template <class Stream>
socket_connection<Stream> create_socket_connection_impl(
    boost::asio::io_context::executor_type executor,
    boost::asio::ssl::context&,
    std::false_type // is ssl stream
)
{
    return socket_connection<Stream>(executor);
}

template <class Stream>
socket_connection<Stream> create_socket_connection(
    boost::asio::io_context::executor_type executor,
    boost::asio::ssl::context& ssl_ctx
)
{
    return create_socket_connection_impl<Stream>(
        executor, 
        ssl_ctx, 
        std::integral_constant<bool, supports_ssl<Stream>()>()
    );
}

} // test
} // mysql
} // boost


#endif
