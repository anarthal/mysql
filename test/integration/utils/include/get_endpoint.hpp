//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_UTILS_INCLUDE_GET_ENDPOINT_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_UTILS_INCLUDE_GET_ENDPOINT_HPP

#include "er_endpoint.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <stdexcept>
#include <utility>

namespace boost {
namespace mysql {
namespace test {

template <class Protocol>
struct endpoint_getter;

template <>
struct endpoint_getter<boost::asio::ip::tcp>
{
    boost::asio::ip::tcp::endpoint operator()(er_endpoint kind)
    {
        if (kind == er_endpoint::localhost)
        {
            return boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 3306);
        }
        else if (kind == er_endpoint::inexistent)
        {
            return boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 45678);
        }
        else
        {
            throw std::invalid_argument("er_endpoint");
        }
    }
};

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
template <>
struct endpoint_getter<boost::asio::local::stream_protocol>
{
    boost::asio::local::stream_protocol::endpoint operator()(er_endpoint kind)
    {
        if (kind == er_endpoint::localhost)
        {
            return boost::asio::local::stream_protocol::endpoint("/var/run/mysqld/mysqld.sock");
        }
        else if (kind == er_endpoint::inexistent)
        {
            return boost::asio::local::stream_protocol::endpoint("/tmp/this/endpoint/does/not/exist");
        }
        else
        {
            throw std::invalid_argument("er_endpoint");
        }
    }
};
#endif

template <class Stream>
typename Stream::lowest_layer_type::endpoint_type get_endpoint(
    er_endpoint kind
)
{
    return endpoint_getter<typename Stream::lowest_layer_type::protocol_type>()(kind);
}


}
}
}

#endif
