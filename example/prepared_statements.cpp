//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//[example_prepared_statements

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

    // Connection parameters
    boost::asio::ip::tcp::endpoint ep (
        boost::asio::ip::address_v4::loopback(), // host
        boost::mysql::default_port                         // port
    );
    boost::mysql::connection_params params (
        argv[1],               // username
        argv[2],               // password
        "boost_mysql_examples" // database to use; leave empty or omit the parameter for no database
    );


    // Declare the connection object and authenticate to the server
    // We use SSL because MySQL 8+ default settings require it.
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx (boost::asio::ssl::context::tls_client);
    boost::mysql::tcp_ssl_connection conn (ctx, ssl_ctx);
    conn.connect(ep, params);

    /**
     * We can tell MySQL to prepare a statement using connection::prepare_statement.
     * We provide a string SQL statement, which can include any number of parameters,
     * identified by question marks. Parameters are optional: you can prepare a statement
     * with no parameters.
     *
     * Prepared statements are stored in the server on a per-connection basis.
     * Once a connection is closed, all prepared statements for that connection are deallocated.
     *
     * The result of prepare_statement is a mysql::prepared_statement object, which is
     * templatized on the stream type of the connection (tcp_ssl_prepared_statement in our case).
     *
     * We prepare two statements, a SELECT and an UPDATE.
     */
    //[prepared_statements_prepare
    const char* salary_getter_sql = "SELECT salary FROM employee WHERE first_name = ?";
    boost::mysql::tcp_ssl_prepared_statement salary_getter = conn.prepare_statement(salary_getter_sql);
    //]
    ASSERT(salary_getter.num_params() == 1); // num_params() returns the number of parameters (question marks)

    const char* salary_updater_sql = "UPDATE employee SET salary = ? WHERE first_name = ?";
    boost::mysql::tcp_ssl_prepared_statement salary_updater = conn.prepare_statement(salary_updater_sql);
    ASSERT(salary_updater.num_params() == 2);

    /*
     * Once a statement has been prepared, it can be executed as many times as
     * desired, by calling prepared_statement::execute(). execute takes as input a
     * (possibly empty) collection of mysql::value's and returns a resultset.
     * The returned resultset works the same as the one returned by connection::query().
     *
     * The parameters passed to execute() are replaced in order of declaration:
     * the first question mark will be replaced by the first passed parameter,
     * the second question mark by the second parameter and so on. The number
     * of passed parameters must match exactly the number of parameters for
     * the prepared statement.
     *
     * Any collection providing member functions begin() and end() returning
     * forward iterators to mysql::value's is acceptable. We use mysql::make_values(),
     * which creates a std::array with the passed in values converted to mysql::value's.
     * An iterator version of execute() is also available.
     */
    //[prepared_statements_execute
    boost::mysql::tcp_ssl_resultset result = salary_getter.execute(boost::mysql::make_values("Efficient"));
    std::vector<boost::mysql::row> salaries = result.read_all(); // Get all the results
    //]
    ASSERT(salaries.size() == 1);
    double salary = salaries[0].values().at(0).get<double>(); // First row, first column
    std::cout << "The salary before the payrise was: " << salary << std::endl;

    /**
     * Run the update. In this case, we must pass in two parameters.
     * Note that MySQL is flexible in the types passed as parameters.
     * In this case, we are sending the value 35000, which gets converted
     * into a mysql::value with type std::int32_t, while the 'salary'
     * column is declared as a DOUBLE. The MySQL server will do
     * the right thing for us.
     */
    salary_updater.execute(boost::mysql::make_values(35000, "Efficient"));

    /**
     * Execute the select again. We can execute a prepared statement
     * as many times as we want. We do NOT need to call
     * connection::prepare_statement() again.
     */
    result = salary_getter.execute(boost::mysql::make_values("Efficient"));
    salaries = result.read_all();
    ASSERT(salaries.size() == 1);
    salary = salaries[0].values().at(0).get<double>();
    ASSERT(salary == 35000); // Our update took place, and the dev got his pay rise
    std::cout << "The salary after the payrise was: " << salary << std::endl;

    /**
     * Close the statements. Closing a statement deallocates it from the server.
     * Once a statement is closed, trying to execute it will return an error.
     *
     * Closing statements implies communicating with the server and can thus fail.
     *
     * Statements are automatically deallocated once the connection is closed.
     * If you are re-using connection objects and preparing statements over time,
     * you should close() your statements to prevent excessive resource usage.
     * If you are not re-using the connections, or are preparing your statements
     * just once at application startup, there is no need to perform this step.
     */
    salary_updater.close();
    salary_getter.close();

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
