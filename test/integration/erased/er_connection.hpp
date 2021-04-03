//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_plhcgl_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_plhcgl_HPP

#include "network_result.hpp"
#include "../get_endpoint.hpp"
#include "er_resultset.hpp"
#include "er_statement.hpp"
#include <boost/mysql/connection_params.hpp>
#include <memory>


namespace boost {
namespace mysql {
namespace test {

class er_connection
{
public:
    virtual ~er_connection() {}
    virtual bool valid() const = 0;
    virtual bool uses_ssl() const = 0;
    virtual bool is_open() const = 0;
    virtual network_result<no_result> physical_connect(endpoint_kind) = 0;
    virtual network_result<no_result> connect(endpoint_kind, const connection_params&) = 0;
    virtual network_result<no_result> handshake(const connection_params&) = 0;
    virtual network_result<er_resultset_ptr> query(boost::string_view query) = 0;
    virtual network_result<er_statement_ptr> prepare_statement(boost::string_view statement) = 0;
    virtual network_result<no_result> quit() = 0;
    virtual network_result<no_result> close() = 0;
};

using er_connection_ptr = std::unique_ptr<er_connection>;

} // test
} // mysql
} // boost

#endif /* TEST_INTEGRATION_NETWORK_FUNCTIONS_HPP_ */
