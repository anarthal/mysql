//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_plah8_IMPL_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_plah8_IMPL_HPP

#include "network_functions.hpp"
#include <boost/asio/use_future.hpp>

namespace boost {
namespace mysql {
namespace test {

// Helpers
template <class Callable>
auto future_impl_errinfo(
    Callable&& cb
) -> network_result<decltype(cb(std::declval<error_info&>()).get())>
{
    using R = decltype(cb(std::declval<error_info&>()).get()); // Callable returns a future<R>
    error_info info ("Error info was not clearer properly");
    std::future<R> fut = cb(info);
    try
    {
        R res = fut.get();
        return network_result<R>(
            error_code(),
            std::move(info),
            std::move(res)
        );
    }
    catch (const boost::system::system_error& err)
    {
        return network_result<R>(err.code(), std::move(info));
    }
}

template <class Callable>
network_result<no_result> future_impl_no_result_errinfo(
    Callable&& cb
)
{
    error_info info ("Error info was not clearer properly");
    std::future<void> fut = cb(info);
    try
    {
        fut.get();
        return network_result<no_result>(error_code(), std::move(info));
    }
    catch (const boost::system::system_error& err)
    {
        return network_result<no_result>(err.code(), std::move(info));
    }
}

template <class Callable>
auto future_impl_noerrinfo(
    Callable&& cb
) -> network_result<decltype(cb().get())>
{
    using R = decltype(cb().get()); // Callable returns a future<R>
    std::future<R> fut = cb();
    try
    {
        return network_result<R>(
            error_code(),
            fut.get()
        );
    }
    catch (const boost::system::system_error& err)
    {
        return network_result<R>(err.code());
    }
}

template <class Callable>
network_result<no_result> future_impl_no_result_noerrinfo(
    Callable&& cb
)
{
    std::future<void> fut = cb();
    try
    {
        fut.get();
        return network_result<no_result>(error_code());
    }
    catch (const boost::system::system_error& err)
    {
        return network_result<no_result>(err.code());
    }
}

// Base templates (no default completion tokens)
template <class Stream>
class async_future_errinfo : public network_functions<Stream>
{
public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override { return "async_future_errinfo"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return future_impl_no_result_errinfo([&] (error_info& output_info) {
            return conn.async_connect(ep, params, output_info, boost::asio::use_future);
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return future_impl_no_result_errinfo([&] (error_info& output_info) {
            return conn.async_handshake(params, output_info, boost::asio::use_future);
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return conn.async_query(query, output_info, boost::asio::use_future);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return conn.async_prepare_statement(statement, output_info, boost::asio::use_future);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return stmt.async_execute(params, output_info, boost::asio::use_future);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return stmt.async_execute(values, output_info, boost::asio::use_future);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return future_impl_no_result_errinfo([&] (error_info& output_info) {
            return stmt.async_close(output_info, boost::asio::use_future);
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return r.async_read_one(output, output_info, boost::asio::use_future);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return r.async_read_many(count, output_info, boost::asio::use_future);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return future_impl_errinfo([&] (error_info& output_info) {
            return r.async_read_all(output_info, boost::asio::use_future);
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return future_impl_no_result_errinfo([&] (error_info& output_info) {
            return conn.async_quit(output_info, boost::asio::use_future);
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return future_impl_no_result_errinfo([&] (error_info& output_info) {
            return conn.async_close(output_info, boost::asio::use_future);
        });
    }
};

template <class Stream>
class async_future_noerrinfo : public network_functions<Stream>
{
public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override { return "async_future_noerrinfo"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return future_impl_no_result_noerrinfo([&] {
            return conn.async_connect(ep, params, boost::asio::use_future);
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return future_impl_no_result_noerrinfo([&] {
            return conn.async_handshake(params, boost::asio::use_future);
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return future_impl_noerrinfo([&] {
            return conn.async_query(query, boost::asio::use_future);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return future_impl_noerrinfo([&]{
            return conn.async_prepare_statement(statement, boost::asio::use_future);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return future_impl_noerrinfo([&]{
            return stmt.async_execute(params, boost::asio::use_future);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return future_impl_noerrinfo([&] {
            return stmt.async_execute(values, boost::asio::use_future);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return future_impl_no_result_noerrinfo([&] {
            return stmt.async_close(boost::asio::use_future);
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return future_impl_noerrinfo([&] {
            return r.async_read_one(output, boost::asio::use_future);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return future_impl_noerrinfo([&] {
            return r.async_read_many(count, boost::asio::use_future);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return future_impl_noerrinfo([&] {
            return r.async_read_all(boost::asio::use_future);
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return future_impl_no_result_noerrinfo([&] {
            return conn.async_quit(boost::asio::use_future);
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return future_impl_no_result_noerrinfo([&] {
            return conn.async_close(boost::asio::use_future);
        });
    }
};

// Default completion tokens
template <>
class async_future_errinfo<tcp_ssl_future_socket> : public network_functions<tcp_ssl_future_socket>
{
public:
    using connection_type = typename network_functions<tcp_ssl_future_socket>::connection_type;
    using prepared_statement_type = typename network_functions<tcp_ssl_future_socket>::prepared_statement_type;
    using resultset_type = typename network_functions<tcp_ssl_future_socket>::resultset_type;

    const char* name() const override { return "async_future_errinfo"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename tcp_ssl_future_socket::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override;
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override;
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override;
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override;
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override;
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override;
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override;
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override;
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override;
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override;
    network_result<no_result> quit(
        connection_type& conn
    ) override;
    network_result<no_result> close(
        connection_type& conn
    ) override;
};

template <>
class async_future_noerrinfo<tcp_ssl_future_socket> : public network_functions<tcp_ssl_future_socket>
{
public:
    using connection_type = typename network_functions<tcp_ssl_future_socket>::connection_type;
    using prepared_statement_type = typename network_functions<tcp_ssl_future_socket>::prepared_statement_type;
    using resultset_type = typename network_functions<tcp_ssl_future_socket>::resultset_type;

    const char* name() const override { return "async_future_noerrinfo"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename tcp_ssl_future_socket::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override;
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override;
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override;
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override;
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override;
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override;
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override;
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override;
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override;
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override;
    network_result<no_result> quit(
        connection_type& conn
    ) override;
    network_result<no_result> close(
        connection_type& conn
    ) override;
};

BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(async_future_errinfo)
BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(async_future_noerrinfo)

} // test
} // mysql
} // boost


#endif