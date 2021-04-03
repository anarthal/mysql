//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "async_future.hpp"

using namespace boost::mysql::test;
using boost::mysql::row;

// We can't extern-template and specialize together; we have to treat
// this specialization as a regular class

// with err info
network_result<no_result> async_future_errinfo<tcp_ssl_future_socket>::connect(
    connection_type& conn,
    const typename tcp_ssl_future_socket::lowest_layer_type::endpoint_type& ep,
    const connection_params& params
)
{
    return future_impl_no_result_errinfo([&] (error_info& output_info) {
        return conn.async_connect(ep, params, output_info);
    });
}

network_result<no_result> async_future_errinfo<tcp_ssl_future_socket>::handshake(
    connection_type& conn,
    const connection_params& params
)
{
    return future_impl_no_result_errinfo([&] (error_info& output_info) {
        return conn.async_handshake(params, output_info);
    });
}

network_result<async_future_errinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_errinfo<tcp_ssl_future_socket>::query(
    connection_type& conn,
    boost::string_view query
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return conn.async_query(query, output_info);
    });
}

network_result<async_future_errinfo<tcp_ssl_future_socket>::prepared_statement_type> 
async_future_errinfo<tcp_ssl_future_socket>::prepare_statement(
    connection_type& conn,
    boost::string_view statement
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return conn.async_prepare_statement(statement, output_info);
    });
}

network_result<async_future_errinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_errinfo<tcp_ssl_future_socket>::execute_statement(
    prepared_statement_type& stmt,
    const execute_params<value_list_it>& params
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return stmt.async_execute(params, output_info);
    });
}

network_result<async_future_errinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_errinfo<tcp_ssl_future_socket>::execute_statement(
    prepared_statement_type& stmt,
    const std::vector<value>& values
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return stmt.async_execute(values, output_info);
    });
}

network_result<no_result> async_future_errinfo<tcp_ssl_future_socket>::close_statement(
    prepared_statement_type& stmt
)
{
    return future_impl_no_result_errinfo([&] (error_info& output_info) {
        return stmt.async_close(output_info);
    });
}

network_result<bool> async_future_errinfo<tcp_ssl_future_socket>::read_one(
    resultset_type& r,
    row& output
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return r.async_read_one(output, output_info);
    });
}

network_result<std::vector<row>> async_future_errinfo<tcp_ssl_future_socket>::read_many(
    resultset_type& r,
    std::size_t count
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return r.async_read_many(count, output_info);
    });
}

network_result<std::vector<row>> async_future_errinfo<tcp_ssl_future_socket>::read_all(
    resultset_type& r
)
{
    return future_impl_errinfo([&] (error_info& output_info) {
        return r.async_read_all(output_info);
    });
}

network_result<no_result> async_future_errinfo<tcp_ssl_future_socket>::quit(
    connection_type& conn
)
{
    return future_impl_no_result_errinfo([&] (error_info& output_info) {
        return conn.async_quit(output_info);
    });
}

network_result<no_result> async_future_errinfo<tcp_ssl_future_socket>::close(
    connection_type& conn
)
{
    return future_impl_no_result_errinfo([&] (error_info& output_info) {
        return conn.async_close(output_info);
    });
}

// without err info
network_result<no_result> async_future_noerrinfo<tcp_ssl_future_socket>::connect(
    connection_type& conn,
    const typename tcp_ssl_future_socket::lowest_layer_type::endpoint_type& ep,
    const connection_params& params
)
{
    return future_impl_no_result_noerrinfo([&] {
        return conn.async_connect(ep, params);
    });
}
network_result<no_result> async_future_noerrinfo<tcp_ssl_future_socket>::handshake(
    connection_type& conn,
    const connection_params& params
)
{
    return future_impl_no_result_noerrinfo([&] {
        return conn.async_handshake(params);
    });
}
network_result<async_future_noerrinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_noerrinfo<tcp_ssl_future_socket>::query(
    connection_type& conn,
    boost::string_view query
)
{
    return future_impl_noerrinfo([&] {
        return conn.async_query(query);
    });
}
network_result<async_future_noerrinfo<tcp_ssl_future_socket>::prepared_statement_type> 
async_future_noerrinfo<tcp_ssl_future_socket>::prepare_statement(
    connection_type& conn,
    boost::string_view statement
)
{
    return future_impl_noerrinfo([&]{
        return conn.async_prepare_statement(statement);
    });
}
network_result<async_future_noerrinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_noerrinfo<tcp_ssl_future_socket>::execute_statement(
    prepared_statement_type& stmt,
    const execute_params<value_list_it>& params
)
{
    return future_impl_noerrinfo([&]{
        return stmt.async_execute(params);
    });
}
network_result<async_future_noerrinfo<tcp_ssl_future_socket>::resultset_type> 
async_future_noerrinfo<tcp_ssl_future_socket>::execute_statement(
    prepared_statement_type& stmt,
    const std::vector<value>& values
)
{
    return future_impl_noerrinfo([&] {
        return stmt.async_execute(values);
    });
}
network_result<no_result> async_future_noerrinfo<tcp_ssl_future_socket>::close_statement(
    prepared_statement_type& stmt
)
{
    return future_impl_no_result_noerrinfo([&] {
        return stmt.async_close();
    });
}
network_result<bool> async_future_noerrinfo<tcp_ssl_future_socket>::read_one(
    resultset_type& r,
    row& output
)
{
    return future_impl_noerrinfo([&] {
        return r.async_read_one(output);
    });
}
network_result<std::vector<row>> async_future_noerrinfo<tcp_ssl_future_socket>::read_many(
    resultset_type& r,
    std::size_t count
)
{
    return future_impl_noerrinfo([&] {
        return r.async_read_many(count);
    });
}
network_result<std::vector<row>> async_future_noerrinfo<tcp_ssl_future_socket>::read_all(
    resultset_type& r
)
{
    return future_impl_noerrinfo([&] {
        return r.async_read_all();
    });
}
network_result<no_result> async_future_noerrinfo<tcp_ssl_future_socket>::quit(
    connection_type& conn
)
{
    return future_impl_no_result_noerrinfo([&] {
        return conn.async_quit();
    });
}
network_result<no_result> async_future_noerrinfo<tcp_ssl_future_socket>::close(
    connection_type& conn
)
{
    return future_impl_no_result_noerrinfo([&] {
        return conn.async_close();
    });
}