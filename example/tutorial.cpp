//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ssl/context.hpp>
#include <boost/mysql.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system/system_error.hpp>
#include <iostream>

/**
 * For this example, we will be using the 'boost_mysql_examples' database.
 * You can get this database by running db_setup.sql.
 * This example assumes you are connecting to a localhost MySQL server.
 *
 * This example uses synchronous functions and handles errors using exceptions.
 */

void main_impl(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <username> <password>\n";
        exit(1);
    }

    //[tutorial_connection
    // The execution context, required to run I/O operations
    boost::asio::io_context ctx;

    // The SSL context, required to establish TLS connections.
    // The default SSL options are good enough for us at this point.
    boost::asio::ssl::context ssl_ctx (boost::asio::ssl::context::tls_client);

    // The object defining the connection to the MySQL server.
    boost::mysql::tcp_ssl_connection conn (ctx.get_executor(), ssl_ctx);
    //]

    //[tutorial_connect
    boost::asio::ip::tcp::endpoint ep (
        boost::asio::ip::address_v4::loopback(), // host
        boost::mysql::default_port               // port
    );

    boost::mysql::connection_params params (
        argv[1], // username
        argv[2]  // password
    );

    conn.connect(ep, params);
    //]


    //[tutorial_query
    const char* sql = "SELECT \"Hello world!\"";
    boost::mysql::tcp_ssl_resultset result = conn.query(sql);
    //]

    //[tutorial_read
    std::vector<boost::mysql::row> employees = result.read_all();
    //]

    //[tutorial_values
    const boost::mysql::row& first_row = employees.at(0);
    boost::mysql::value first_value = first_row.values().at(0);
    std::cout << first_value << std::endl;
    //]

    //[tutorial_close
    conn.close();
    //]
}

int main(int argc, char** argv)
{
    try
    {
        main_impl(argc, argv);
    }
    catch (const boost::system::system_error& err)
    {
        std::cerr << "Error: " << err.what() << ", error code: " << err.code() << std::endl;
        return 1;
    }
    catch (const std::exception& err)
    {
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
}
