//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "er_network_variant.hpp"
#include "er_connection.hpp"
#include "er_resultset.hpp"
#include "er_statement.hpp"
#include "network_result.hpp"
#include "streams.hpp"
#include "er_impl_common.hpp"
#include "get_endpoint.hpp"
#include "boost/mysql/connection_params.hpp"
#include "boost/mysql/errc.hpp"
#include "boost/mysql/error.hpp"
#include "boost/mysql/execute_params.hpp"
#include "boost/mysql/prepared_statement.hpp"
#include "boost/mysql/row.hpp"
#include <boost/mysql/resultset.hpp>
#include <boost/mysql/socket_connection.hpp>
#include <memory>

using namespace boost::mysql::test;
using boost::mysql::row;
using boost::mysql::error_code;
using boost::mysql::error_info;
using boost::mysql::value;
using boost::mysql::connection_params;

namespace {

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

template <class Stream>
class sync_exc_resultset : public er_resultset_base<Stream>
{
public:
    using er_resultset_base<Stream>::er_resultset_base;
    network_result<bool> read_one(row& output) override
    {
        return impl([&] {
            return this->r_.read_one(output);
        });
    }
    network_result<std::vector<row>> read_many(std::size_t count) override
    {
        return impl([&] {
            return this->r_.read_many(count);
        });
    }
    network_result<std::vector<row>> read_all() override
    {
        return impl([&] {
            return this->r_.read_all();
        });
    }
};

template <class Stream>
class sync_exc_statement : public er_statement_base<Stream>
{
public:
    using er_statement_base<Stream>::er_statement_base;
    network_result<er_resultset_ptr> execute_params(
        const boost::mysql::execute_params<value_list_it>& params
    ) override
    {
        return impl([&]{
            return erase_resultset<sync_exc_resultset>(this->stmt_.execute(params));
        });
    }
    network_result<er_resultset_ptr> execute_container(
        const std::vector<value>& values
    ) override
    {
        return impl([&] {
            return erase_resultset<sync_exc_resultset>(this->stmt_.execute(values));
        });
    }
    network_result<no_result> close() override
    {
        return impl([&] {
            this->stmt_.close();
            return no_result();
        });
    }
};

template <class Stream>
class sync_exc_connection : public er_connection_base<Stream>
{
public:
    using er_connection_base<Stream>::er_connection_base;
    network_result<no_result> physical_connect(er_endpoint kind) override
    {
        return impl([&] {
            this->conn_.next_layer().lowest_layer().connect(get_endpoint<Stream>(kind));
            return no_result();
        });
    }
    network_result<no_result> connect(
        er_endpoint kind,
        const boost::mysql::connection_params& params
    ) override
    {
        return impl([&] {
            this->conn_.connect(get_endpoint<Stream>(kind), params);
            return no_result();
        });
    }
    network_result<no_result> handshake(const connection_params& params) override
    {
        return impl([&] {
            this->conn_.handshake(params);
            return no_result();
        });
    }
    network_result<er_resultset_ptr> query(boost::string_view query) override
    {
        return impl([&] {
            return erase_resultset<sync_exc_resultset>(this->conn_.query(query));
        });
    }
    network_result<er_statement_ptr> prepare_statement(boost::string_view statement) override
    {
        return impl([&] {
            return erase_statement<sync_exc_statement>(this->conn_.prepare_statement(statement));
        });
    }
    network_result<no_result> quit() override
    {
        return impl([&] {
            this->conn_.quit();
            return no_result();
        });
    }
    network_result<no_result> close() override
    {
        return impl([&] {
            this->conn_.close();
            return no_result();
        });
    }
};

template <class Stream>
class sync_exc_variant : public er_network_variant_base<Stream, sync_exc_connection>
{
public:
    const char* variant_name() const override { return "sync_exc"; }
};

sync_exc_variant<tcp_socket> tcp;
sync_exc_variant<tcp_ssl_socket> tcp_ssl;
// UNIX sockets don't add much value here

} // anon namespace

void boost::mysql::test::add_sync_exc(
    std::vector<er_network_variant*>& output
)
{
    output.push_back(&tcp);
    output.push_back(&tcp_ssl);
}