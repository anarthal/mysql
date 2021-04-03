//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_plch19_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_plch19_HPP

#include "er_connection.hpp"
#include <boost/asio/ssl/context.hpp>
#include <functional>
#include <boost/asio/io_context.hpp>

namespace boost {
namespace mysql {
namespace test {

class network_variant
{
public:
    virtual ~network_variant() {}
    virtual bool supports_ssl() const = 0;
    virtual const char* stream_name() const = 0;
    virtual const char* variant_name() const = 0;
    virtual er_connection_ptr create(boost::asio::io_context::executor_type, boost::asio::ssl::context&) = 0;
};

const std::vector<network_variant*>& all_variants();
const std::vector<network_variant*>& ssl_variants();
const std::vector<network_variant*>& non_ssl_variants();
network_variant* tcp_sync_errc_variant();

void add_sync_errc(std::vector<network_variant*>&);

} // test
} // mysql
} // boost


#endif
