//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_INTEGRATION_TEST_COMMON_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_INTEGRATION_TEST_COMMON_HPP

#include <boost/asio/ssl/context.hpp>
#include <boost/mysql/socket_connection.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/test/unit_test.hpp>
#include <thread>
#include <type_traits>
#include "boost/mysql/connection_params.hpp"
#include "test_common.hpp"
#include "metadata_validator.hpp"
#include "network_functions.hpp"
#include "get_endpoint.hpp"
#include "network_test.hpp"
#include "stream_list.hpp"

namespace boost {
namespace mysql {
namespace test {

// Verifies that we are or are not using SSL, depending on whether the stream supports it or not
template <class Stream>
void validate_ssl(const connection<Stream>& conn, ssl_mode m)
{
    bool expected = (m == ssl_mode::require || m == ssl_mode::enable) && supports_ssl<Stream>();
    BOOST_TEST(conn.uses_ssl() == expected);
}

// Helper to create a socket_connection<Stream> from a SSL context;
// also usable for non-SSL streams
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


/**
 * Base fixture to use in integration tests. The fixture constructor creates
 * a connection, an asio io_context and a thread to run it.
 * The fixture is template-parameterized by a stream type, as required
 * by BOOST_MYSQL_NETWORK_TEST.
 */
template <class Stream>
struct network_fixture
{
    using stream_type = Stream;

    connection_params params;
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    socket_connection<Stream> conn;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard;
    std::thread runner;

    network_fixture() :
        params("integ_user", "integ_password", "boost_mysql_integtests"),
        ssl_ctx(boost::asio::ssl::context::tls_client),
        conn(create_socket_connection<Stream>(ctx.get_executor(), ssl_ctx)),
        guard(ctx.get_executor()),
        runner([this] { ctx.run(); })
    {
    }

    ~network_fixture()
    {
        error_code code;
        error_info info;
        conn.close(code, info);
        guard.reset();
        runner.join();
    }

    void set_credentials(boost::string_view user, boost::string_view password)
    {
        params.set_username(user);
        params.set_password(password);
    }

    void physical_connect()
    {
        conn.next_layer().lowest_layer().connect(get_endpoint<Stream>(endpoint_kind::localhost));
    }

    // Used by tests that modify the SSL context parameters
    void recreate_connection()
    {
        this->conn = create_socket_connection<Stream>(ctx.get_executor(), ssl_ctx);
    }

    void handshake(ssl_mode m = ssl_mode::require)
    {
        params.set_ssl(m);
        conn.handshake(params);
        validate_ssl(conn, m);
    }

    void connect(ssl_mode m = ssl_mode::require)
    {
        physical_connect();
        handshake(m);
    }

    void validate_2fields_meta(
        const std::vector<field_metadata>& fields,
        const std::string& table
    ) const
    {
        validate_meta(fields, {
            meta_validator(table, "id", field_type::int_),
            meta_validator(table, "field_varchar", field_type::varchar)
        });
    }

    void validate_2fields_meta(
        const resultset<Stream>& result,
        const std::string& table
    ) const
    {
        validate_2fields_meta(result.fields(), table);
    }

    // Call this in the fixture setup of any test invoking write
    // operations on the database, to prevent race conditions,
    // make the testing environment more stable and speed up the tests
    void start_transaction()
    {
        this->conn.query("START TRANSACTION").read_all();
    }

    std::int64_t get_table_size(const std::string& table)
    {
        return this->conn.query("SELECT COUNT(*) FROM " + table)
                .read_all().at(0).values().at(0).template get<std::int64_t>();
    }
};

// To be used as sample in data driven tests, when a test case should be run
// over all different network_function's.
template <class Stream>
struct network_sample
{
    network_functions<Stream>* net;

    network_sample(network_functions<Stream>* funs) :
        net(funs)
    {
    }

    void set_test_attributes(boost::unit_test::test_case& test) const
    {
        test.add_label(net->name());
    }
};

template <class Stream>
std::ostream& operator<<(std::ostream& os, const network_sample<Stream>& value)
{
    return os << value.net->name();
}

// Data generator for network_sample
struct network_gen
{
    template <class Stream>
    static std::vector<network_sample<Stream>> make_all()
    {
        std::vector<network_sample<Stream>> res;
        for (auto* net: all_network_functions<Stream>())
        {
            res.emplace_back(net);
        }
        return res;
    }

    template <class Stream>
    static const std::vector<network_sample<Stream>>& generate()
    {
        static std::vector<network_sample<Stream>> res = make_all<Stream>();
        return res;
    }
};


} // test
} // mysql
} // boost

#endif /* TEST_INTEGRATION_INTEGRATION_TEST_COMMON_HPP_ */
