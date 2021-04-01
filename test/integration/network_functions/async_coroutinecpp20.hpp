//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_ASYNC_COROUTINECPP20_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_ASYNC_COROUTINECPP20_HPP

#ifdef BOOST_ASIO_HAS_CO_AWAIT

#include "network_functions.hpp"
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <future>

namespace boost {
namespace mysql {
namespace test {

template <class Stream>
class async_coroutinecpp20_errinfo : public network_functions<Stream>
{
    template <class Callable>
    using impl_result_type = typename decltype(std::declval<Callable>()(
        std::declval<error_info&>()
    ))::value_type;

    template <class IoObj, class Callable>
    network_result<impl_result_type<Callable>> impl(IoObj& obj, Callable&& cb) {
        using R = impl_result_type<Callable>;
        std::promise<network_result<R>> prom;

        boost::asio::co_spawn(obj.get_executor(), [&] () -> boost::asio::awaitable<void> {
            error_info info ("error_info not cleared properly");
            try
            {
                R result = co_await cb(info);
                prom.set_value(network_result<R>(error_code(), std::move(info), std::move(result)));
            }
            catch (const boost::system::system_error& err)
            {
                prom.set_value(network_result<R>(err.code(), std::move(info)));
            }
        }, boost::asio::detached);

        return prom.get_future().get();
    }

    template <class IoObj, class Callable>
    network_result<no_result> impl_no_result(IoObj& obj, Callable&& cb) {
        std::promise<network_result<no_result>> prom;

        boost::asio::co_spawn(obj.get_executor(), [&] () -> boost::asio::awaitable<void> {
            error_info info ("error_info not cleared properly");
            try
            {
                co_await cb(info);
                prom.set_value(network_result<no_result>(error_code(), std::move(info)));
            }
            catch (const boost::system::system_error& err)
            {
                prom.set_value(network_result<no_result>(err.code(), std::move(info)));
            }
        }, boost::asio::detached);

        return prom.get_future().get();
    }

public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override
    {
        return "async_coroutinecpp20_errinfo";
    }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return impl_no_result(conn, [&](error_info& info) {
            return conn.async_connect(ep, params, info, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return impl_no_result(conn, [&](error_info& info) {
            return conn.async_handshake(params, info, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return impl(conn, [&](error_info& info) {
            return conn.async_query(query, info, boost::asio::use_awaitable);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return impl(conn, [&](error_info& info) {
            return conn.async_prepare_statement(statement, info, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return impl(stmt, [&](error_info& info) {
            return stmt.async_execute(params, info, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return impl(stmt, [&](error_info& info) {
            return stmt.async_execute(values, info, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return impl_no_result(stmt, [&](error_info& info) {
            return stmt.async_close(info, boost::asio::use_awaitable);
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return impl(r, [&](error_info& info) {
            return r.async_read_one(output, info, boost::asio::use_awaitable);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return impl(r, [&](error_info& info) {
            return r.async_read_many(count, info, boost::asio::use_awaitable);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return impl(r, [&](error_info& info) {
            return r.async_read_all(info, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return impl_no_result(conn, [&](error_info& info) {
            return conn.async_quit(info, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return impl_no_result(conn, [&](error_info& info) {
            return conn.async_close(info, boost::asio::use_awaitable);
        });
    }
};

template <class Stream>
class async_coroutinecpp20_noerrinfo : public network_functions<Stream>
{
    template <class Callable>
    using impl_result_type = typename decltype(std::declval<Callable>()())::value_type;

    template <class IoObj, class Callable>
    network_result<impl_result_type<Callable>> impl(IoObj& obj, Callable&& cb)
    {
        using R = impl_result_type<Callable>;
        std::promise<network_result<R>> prom;

        boost::asio::co_spawn(obj.get_executor(), [&] () -> boost::asio::awaitable<void> {
            try
            {
                prom.set_value(network_result<R>(error_code(), co_await cb()));
            }
            catch (const boost::system::system_error& err)
            {
                prom.set_value(network_result<R>(err.code()));
            }
        }, boost::asio::detached);

        return prom.get_future().get();
    }

    template <class IoObj, class Callable>
    network_result<no_result> impl_no_result(IoObj& obj, Callable&& cb)
    {
        std::promise<network_result<no_result>> prom;

        boost::asio::co_spawn(obj.get_executor(), [&] () -> boost::asio::awaitable<void> {
            try
            {
                co_await cb();
                prom.set_value(network_result<no_result>(error_code()));
            }
            catch (const boost::system::system_error& err)
            {
                prom.set_value(network_result<no_result>(err.code()));
            }
        }, boost::asio::detached);

        return prom.get_future().get();
    }

public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override
    {
        return "async_coroutinecpp20_noerrinfo";
    }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return impl_no_result(conn, [&] {
            return conn.async_connect(ep, params, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return impl_no_result(conn, [&] {
            return conn.async_handshake(params, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return impl(conn, [&] {
            return conn.async_query(query, boost::asio::use_awaitable);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return impl(conn, [&] {
            return conn.async_prepare_statement(statement, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return impl(stmt, [&] {
            return stmt.async_execute(params, boost::asio::use_awaitable);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return impl(stmt, [&] {
            return stmt.async_execute(values, boost::asio::use_awaitable);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return impl_no_result(stmt, [&] {
            return stmt.async_close(boost::asio::use_awaitable);
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return impl(r, [&] {
            return r.async_read_one(output, boost::asio::use_awaitable);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return impl(r, [&] {
            return r.async_read_many(count, boost::asio::use_awaitable);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return impl(r, [&] {
            return r.async_read_all(boost::asio::use_awaitable);
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return impl_no_result(conn, [&] {
            return conn.async_quit(boost::asio::use_awaitable);
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return impl_no_result(conn, [&] {
            return conn.async_close(boost::asio::use_awaitable);
        });
    }
};

BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(async_coroutinecpp20_errinfo)
BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(async_coroutinecpp20_noerrinfo)

} // test
} // mysql
} // boost

#endif // BOOST_ASIO_HAS_CO_AWAIT

#endif
