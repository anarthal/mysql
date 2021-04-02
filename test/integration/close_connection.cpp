//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/mysql/detail/protocol/constants.hpp"
#include "integration_test_common.hpp"
#include "network_test.hpp"

using namespace boost::mysql::test;
using boost::mysql::error_code;

BOOST_AUTO_TEST_SUITE(test_close_connection)

BOOST_MYSQL_NETWORK_TEST(active_connection, network_fixture)
{
    setup(sample.net);

    // Connect
    connect();

    // Close
    conn->close().validate_no_error();

    // We are no longer able to query
    conn->query("SELECT 1").validate_any_error();

    // The stream is closed
    BOOST_TEST(!conn->is_open());
}

BOOST_MYSQL_NETWORK_TEST(double_close, network_fixture)
{
    setup(sample.net);

    // Connect
    connect();

    // Close
    conn->close().validate_no_error();

    // The stream (socket) is closed
    BOOST_TEST(!conn->is_open());

    // Closing again returns OK (and does nothing)
    conn->close().validate_no_error();

    // Stream (socket) still closed
    BOOST_TEST(!conn->is_open());
}

BOOST_MYSQL_NETWORK_TEST(not_open_connection, network_fixture)
{
    setup(sample.net);
    conn->close().validate_no_error();
    BOOST_TEST(!conn->is_open());
}


BOOST_AUTO_TEST_SUITE_END() // test_close_connection
