//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_plah_IMPL_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_plah_IMPL_HPP

#include "network_functions.hpp"

namespace boost {
namespace mysql {
namespace test {

template <class Stream>
class sync_errc : public network_functions<Stream>
{
    template <class Callable>
    using impl_result_type = decltype(std::declval<Callable>()(
        std::declval<error_code&>(),
        std::declval<error_info&>()
    ));

    template <class Callable>
    static network_result<impl_result_type<Callable>> impl(Callable&& cb)
    {
        network_result<impl_result_type<Callable>> res (
            boost::mysql::make_error_code(errc::no),
            error_info("error_info not cleared properly")
        );
        res.value = cb(res.err, *res.info);
        return res;
    }
public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override { return "sync_errc"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn.connect(ep, params, code, info);
            return no_result();
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn.handshake(params, code, info);
            return no_result();
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            return conn.query(query, code, info);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return impl([&conn, statement](error_code& err, error_info& info) {
            return conn.prepare_statement(statement, err, info);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return impl([=, &stmt](error_code& err, error_info& info) {
            return stmt.execute(params, err, info);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return impl([&stmt, &values](error_code& err, error_info& info) {
            return stmt.execute(values, err, info);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            stmt.close(code, info);
            return no_result();
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            return r.read_one(output, code, info);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            return r.read_many(count, code, info);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            return r.read_all(code, info);
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn.quit(code, info);
            return no_result();
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn.close(code, info);
            return no_result();
        });
    }
};

template <class Stream>
class sync_exc : public network_functions<Stream>
{
    template <class Callable>
    using impl_result_type = decltype(std::declval<Callable>()());

    template <class Callable>
    static network_result<impl_result_type<Callable>> impl(Callable&& cb)
    {
        network_result<impl_result_type<Callable>> res;
        try
        {
            res.value = cb();
        }
        catch (const boost::system::system_error& err)
        {
            res.err = err.code();
            res.info = error_info(err.what());
        }
        return res;
    }
public:
    using connection_type = typename network_functions<Stream>::connection_type;
    using prepared_statement_type = typename network_functions<Stream>::prepared_statement_type;
    using resultset_type = typename network_functions<Stream>::resultset_type;

    const char* name() const override { return "sync_exc"; }
    network_result<no_result> connect(
        connection_type& conn,
        const typename Stream::lowest_layer_type::endpoint_type& ep,
        const connection_params& params
    ) override
    {
        return impl([&] {
            conn.connect(ep, params);
            return no_result();
        });
    }
    network_result<no_result> handshake(
        connection_type& conn,
        const connection_params& params
    ) override
    {
        return impl([&] {
            conn.handshake(params);
            return no_result();
        });
    }
    network_result<resultset_type> query(
        connection_type& conn,
        boost::string_view query
    ) override
    {
        return impl([&] {
            return conn.query(query);
        });
    }
    network_result<prepared_statement_type> prepare_statement(
        connection_type& conn,
        boost::string_view statement
    ) override
    {
        return impl([&conn, statement] {
            return conn.prepare_statement(statement);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const execute_params<value_list_it>& params
    ) override
    {
        return impl([&]{
            return stmt.execute(params);
        });
    }
    network_result<resultset_type> execute_statement(
        prepared_statement_type& stmt,
        const std::vector<value>& values
    ) override
    {
        return impl([&stmt, &values] {
            return stmt.execute(values);
        });
    }
    network_result<no_result> close_statement(
        prepared_statement_type& stmt
    ) override
    {
        return impl([&] {
            stmt.close();
            return no_result();
        });
    }
    network_result<bool> read_one(
        resultset_type& r,
		row& output
    ) override
    {
        return impl([&] {
            return r.read_one(output);
        });
    }
    network_result<std::vector<row>> read_many(
        resultset_type& r,
        std::size_t count
    ) override
    {
        return impl([&] {
            return r.read_many(count);
        });
    }
    network_result<std::vector<row>> read_all(
        resultset_type& r
    ) override
    {
        return impl([&] {
            return r.read_all();
        });
    }
    network_result<no_result> quit(
        connection_type& conn
    ) override
    {
        return impl([&] {
            conn.quit();
            return no_result();
        });
    }
    network_result<no_result> close(
        connection_type& conn
    ) override
    {
        return impl([&] {
            conn.close();
            return no_result();
        });
    }
};

BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(sync_errc)
BOOST_MYSQL_NETFN_EXTERN_TEMPLATE(sync_exc)

}
}
}

#endif