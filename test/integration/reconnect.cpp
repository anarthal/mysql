//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/mysql/socket_connection.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "boost/mysql/connection_params.hpp"
#include "boost/mysql/errc.hpp"
#include "get_endpoint.hpp"
#include "integration_test_common.hpp"

using namespace boost::mysql::test;
using boost::mysql::ssl_mode;

BOOST_AUTO_TEST_SUITE(test_reconnect)

template <class Stream>
struct reconnect_fixture : network_fixture<Stream>
{
    using network_fixture<Stream>::network_fixture;

    void do_connect_ok(network_functions<Stream>* net)
    {
        auto result = net->connect(this->conn, get_endpoint<Stream>(endpoint_kind::localhost), this->params);
        result.validate_no_error();
        validate_ssl(this->conn, this->params.ssl());
    }

    void do_query_ok(network_functions<Stream>* net)
    {
        auto result = net->query(this->conn, "SELECT * FROM empty_table");
        result.validate_no_error();
        auto read_result = net->read_all(result.value);
        read_result.validate_no_error();
        BOOST_TEST(read_result.value.empty());
    }
};

BOOST_MYSQL_NETWORK_TEST_EX(reconnect_after_close, reconnect_fixture, network_gen, all_non_ssl_streams)
{
    // Connect and use the connection
    this->do_connect_ok(sample.net);
    this->do_query_ok(sample.net);

    // Close
    auto result = sample.net->close(this->conn);
    result.validate_no_error();

    // Reopen and use the connection normally
    this->do_connect_ok(sample.net);
    this->do_query_ok(sample.net);
}

BOOST_MYSQL_NETWORK_TEST_EX(reconnect_after_handshake_error, reconnect_fixture, network_gen, all_non_ssl_streams)
{
    // Error during server handshake
    this->params.set_database("bad_database");
    auto result = sample.net->connect(this->conn, get_endpoint<Stream>(endpoint_kind::localhost), this->params);
    result.validate_error(boost::mysql::errc::dbaccess_denied_error, {"database", "bad_database"});

    // Reopen with correct parameters and use the connection normally
    this->params.set_database("boost_mysql_integtests");
    this->do_connect_ok(sample.net);
    this->do_query_ok(sample.net);
}

BOOST_MYSQL_NETWORK_TEST(reconnect_after_physical_connect_error, reconnect_fixture, network_gen)
{
    // Error during connection
    auto result = sample.net->connect(this->conn, get_endpoint<Stream>(endpoint_kind::inexistent), this->params);
    result.validate_any_error({"physical connect failed"});

    // Reopen with and use the connection normally
    this->do_connect_ok(sample.net);
    this->do_query_ok(sample.net);
}

// BOOST_MYSQL_NETWORK_TEST_EX(reconnect_after_ssl_handshake_error, reconnect_fixture, network_gen, all_ssl_streams)
// {
//     // Error during SSL certificate validation
//     this->conn.next_layer().set_verify_mode(boost::asio::ssl::verify_peer);
//     auto result = sample.net->connect(this->conn, get_endpoint<Stream>(endpoint_kind::localhost), this->params);
//     BOOST_TEST(result.err.message() == "certificate verify failed");
//     BOOST_TEST(!this->conn.uses_ssl());

//     // Disable certificate validation and use the connection normally
//     this->conn.next_layer().set_verify_mode(boost::asio::ssl::verify_none);
//     this->do_connect_ok(sample.net);
//     BOOST_TEST(this->conn.uses_ssl());
//     this->do_query_ok(sample.net);
// }

BOOST_AUTO_TEST_SUITE_END() // test_reconnect

