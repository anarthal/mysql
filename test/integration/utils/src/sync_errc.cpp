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
using boost::mysql::resultset;
using boost::mysql::row;
using boost::mysql::errc;
using boost::mysql::error_code;
using boost::mysql::error_info;
using boost::mysql::field_metadata;
using boost::mysql::prepared_statement;
using boost::mysql::value;
using boost::mysql::socket_connection;
using boost::mysql::connection_params;

namespace {

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

template <class Stream>
class sync_errc_resultset : public er_resultset
{
    resultset<Stream> r_;
public:
    sync_errc_resultset(resultset<Stream>&& r): r_(std::move(r)) {}
    network_result<bool> read_one(row& output) override
    {
        return impl([&](error_code& code, error_info& info) {
            return r_.read_one(output, code, info);
        });
    }
    network_result<std::vector<row>> read_many(std::size_t count) override
    {
        return impl([&](error_code& code, error_info& info) {
            return r_.read_many(count, code, info);
        });
    }
    network_result<std::vector<row>> read_all() override
    {
        return impl([&](error_code& code, error_info& info) {
            return r_.read_all(code, info);
        });
    }
    bool valid() const override { return r_.valid(); } 
    bool complete() const override { return r_.complete(); }
    const std::vector<field_metadata>& fields() const override { return r_.fields(); }
    std::uint64_t affected_rows() const override { return r_.affected_rows(); }
    std::uint64_t last_insert_id() const override { return r_.last_insert_id(); }
    unsigned warning_count() const override { return r_.warning_count(); }
    boost::string_view info() const override { return r_.info(); }
};

template <class Stream>
class sync_errc_statement : public er_statement
{
    prepared_statement<Stream> stmt_;
public:
    sync_errc_statement(prepared_statement<Stream>&& stmt): stmt_(std::move(stmt)) {}
    network_result<er_resultset_ptr> execute_params(
        const boost::mysql::execute_params<value_list_it>& params
    ) override
    {
        return impl([&](error_code& err, error_info& info) {
            return er_resultset_ptr(new sync_errc_resultset<Stream>(stmt_.execute(params, err, info)));
        });
    }
    network_result<er_resultset_ptr> execute_container(
        const std::vector<value>& values
    ) override
    {
        return impl([&](error_code& err, error_info& info) {
            return er_resultset_ptr(new sync_errc_resultset<Stream>(stmt_.execute(values, err, info)));
        });
    }
    network_result<no_result> close() override
    {
        return impl([&](error_code& code, error_info& info) {
            stmt_.close(code, info);
            return no_result();
        });
    }

    bool valid() const override { return stmt_.valid(); }
    unsigned id() const override { return stmt_.id(); }
    unsigned num_params() const override { return stmt_.num_params(); }
};

template <class Stream>
class sync_errc_connection : public er_connection
{
    socket_connection<Stream> conn_;
public:
    sync_errc_connection(socket_connection<Stream>&& conn) : conn_(std::move(conn)) {}
    network_result<no_result> physical_connect(er_endpoint kind) override
    {
        return impl([&](error_code& code, error_info&) {
            conn_.next_layer().lowest_layer().connect(get_endpoint<Stream>(kind), code);
            return no_result();
        });
    }
    network_result<no_result> connect(
        er_endpoint kind,
        const boost::mysql::connection_params& params
    ) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn_.connect(get_endpoint<Stream>(kind), params, code, info);
            return no_result();
        });
    }
    network_result<no_result> handshake(const connection_params& params) override
    {
        return impl([&](error_code& code, error_info& info) {
            conn_.handshake(params, code, info);
            return no_result();
        });
    }
    network_result<er_resultset_ptr> query(boost::string_view query) override
    {
        return impl([&](error_code& code, error_info& info) {
            return er_resultset_ptr(new sync_errc_resultset<Stream>(conn_.query(query, code, info)));
        });
    }
    network_result<er_statement_ptr> prepare_statement(boost::string_view statement) override
    {
        return impl([&](error_code& err, error_info& info) {
            return er_statement_ptr(new sync_errc_statement<Stream>(conn_.prepare_statement(statement, err, info)));
        });
    }
    network_result<no_result> quit() override
    {
        return impl([&](error_code& code, error_info& info) {
            conn_.quit(code, info);
            return no_result();
        });
    }
    network_result<no_result> close() override
    {
        return impl([&](error_code& code, error_info& info) {
            conn_.close(code, info);
            return no_result();
        });
    }
    bool valid() const override { return conn_.valid(); }
    bool uses_ssl() const override { return conn_.uses_ssl(); }
    bool is_open() const override { return conn_.next_layer().lowest_layer().is_open(); }
};

template <class Stream>
class errc_variant : public network_variant
{
public:
    bool supports_ssl() const override { return ::supports_ssl<Stream>(); }
    const char* stream_name() const override { return ::get_stream_name<Stream>(); }
    const char* variant_name() const override { return "sync_errc"; }
    er_connection_ptr create(boost::asio::io_context::executor_type ex, boost::asio::ssl::context& ssl_ctx) override
    {
        return er_connection_ptr(new sync_errc_connection<Stream>(
            create_socket_connection<Stream>(ex, ssl_ctx)
        ));
    }
};

errc_variant<tcp_socket> tcp;
errc_variant<tcp_ssl_socket> tcp_ssl;
errc_variant<tcp_ssl_future_socket> def_compl;
#if BOOST_ASIO_HAS_LOCAL_SOCKETS
errc_variant<unix_socket> unix;
#endif

} // anon namespace

void boost::mysql::test::add_sync_errc(
    std::vector<network_variant*>& output
)
{
    output.push_back(&tcp);
    output.push_back(&tcp_ssl);
    output.push_back(&def_compl);
#if BOOST_ASIO_HAS_LOCAL_SOCKETS
    output.push_back(&unix);
#endif
}