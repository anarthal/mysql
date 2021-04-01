//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//[example_metadata

#include <boost/asio/ssl/context.hpp>
#include <boost/mysql.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system/system_error.hpp>
#include <iostream>

#define ASSERT(expr) \
    if (!(expr)) \
    { \
        std::cerr << "Assertion failed: " #expr << std::endl; \
        exit(1); \
    }

void main_impl(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <username> <password>\n";
        exit(1);
    }

    // Connection params (host, port, user, password, database)
    boost::asio::ip::tcp::endpoint ep (boost::asio::ip::address_v4::loopback(), boost::mysql::default_port);
    boost::mysql::connection_params params (argv[1], argv[2], "boost_mysql_examples");

    // TCP and MySQL level connect. We use SSL because MySQL 8+ default settings require it.
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx (boost::asio::ssl::context::tls_client);
    boost::mysql::tcp_ssl_connection conn (ctx, ssl_ctx);
    conn.connect(ep, params);

    // Issue the query
    const char* sql = R"(
        SELECT comp.name AS company_name, emp.id AS employee_id
        FROM employee emp
        JOIN company comp ON (comp.id = emp.company_id)
    )";
    boost::mysql::tcp_ssl_resultset result = conn.query(sql);

    /**
     * Resultsets allow you to access metadata about the fields in the query
     * using the fields() function, which returns a vector of field_metadata
     * (one per field in the query, and in the same order as in the query).
     * You can retrieve the field name, type, number of decimals,
     * suggested display width, whether the field is part of a key...
     */
    ASSERT(result.fields().size() == 2);

    const boost::mysql::field_metadata& company_name = result.fields()[0];
    ASSERT(company_name.database() == "boost_mysql_examples"); // database name
    ASSERT(company_name.table() == "comp");                    // the alias we assigned to the table in the query
    ASSERT(company_name.original_table() == "company");        // the original table name
    ASSERT(company_name.field_name() == "company_name");       // the name of the field in the query
    ASSERT(company_name.original_field_name() == "name");      // the name of the physical field in the table
    ASSERT(company_name.type() == boost::mysql::field_type::varchar); // we created the field as a VARCHAR
    ASSERT(!company_name.is_primary_key());                    // field is not a primary key
    ASSERT(!company_name.is_auto_increment());                 // field is not AUTO_INCREMENT
    ASSERT(company_name.is_not_null());                        // field may not be NULL

    const boost::mysql::field_metadata& employee_id = result.fields()[1];
    ASSERT(employee_id.database() == "boost_mysql_examples"); // database name
    ASSERT(employee_id.table() == "emp");                   // the alias we assigned to the table in the query
    ASSERT(employee_id.original_table() == "employee");     // the original table name
    ASSERT(employee_id.field_name() == "employee_id");      // the name of the field in the query
    ASSERT(employee_id.original_field_name() == "id");      // the name of the physical field in the table
    ASSERT(employee_id.type() == boost::mysql::field_type::int_);  // we created the field as INT
    ASSERT(employee_id.is_primary_key());                   // field is a primary key
    ASSERT(employee_id.is_auto_increment());                // we declared the field as AUTO_INCREMENT
    ASSERT(employee_id.is_not_null());                      // field cannot be NULL

    // Close the connection
    conn.close();
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

//]

