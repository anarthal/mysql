//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "integration_test_common.hpp"

using namespace boost::mysql::test;
using boost::mysql::value;

BOOST_AUTO_TEST_SUITE(test_prepared_statement_lifecycle)

template <class Stream>
value get_updates_table_value(
    boost::mysql::connection<Stream>& conn,
    const std::string& field_varchar="f0"
)
{
    return conn.query(
        "SELECT field_int FROM updates_table WHERE field_varchar = '" + field_varchar + "'")
            .read_all().at(0).values().at(0);
}

BOOST_MYSQL_NETWORK_TEST(select_with_parameters_multiple_executions, network_fixture, network_gen)
{
    this->connect();
    auto* net = sample.net;

    // Prepare a statement
    auto stmt = net->prepare_statement(
        this->conn,
        "SELECT * FROM two_rows_table WHERE id = ? OR field_varchar = ?"
    );
    stmt.validate_no_error();

    // Execute it. Only one row will be returned (because of the id)
    auto result = net->execute_statement(stmt.value, make_value_vector(1, "non_existent"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(!result.value.complete());
    this->validate_2fields_meta(result.value, "two_rows_table");

    auto rows = net->read_all(result.value);
    rows.validate_no_error();
    BOOST_TEST_REQUIRE(rows.value.size() == 1);
    BOOST_TEST((rows.value[0] == makerow(1, "f0")));
    BOOST_TEST(result.value.complete());

    // Execute it again, but with different values. This time, two rows are returned
    result = net->execute_statement(stmt.value, make_value_vector(1, "f1"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(!result.value.complete());
    this->validate_2fields_meta(result.value, "two_rows_table");

    rows = net->read_all(result.value);
    rows.validate_no_error();
    BOOST_TEST_REQUIRE(rows.value.size() == 2);
    BOOST_TEST((rows.value[0] == makerow(1, "f0")));
    BOOST_TEST((rows.value[1] == makerow(2, "f1")));
    BOOST_TEST(result.value.complete());

    // Close it
    auto close_result = net->close_statement(stmt.value);
    close_result.validate_no_error();
}

BOOST_MYSQL_NETWORK_TEST(insert_with_parameters_multiple_executions, network_fixture, network_gen)
{
    this->connect();
    this->start_transaction();
    auto* net = sample.net;

    // Prepare a statement
    auto stmt = net->prepare_statement(
        this->conn,
        "INSERT INTO inserts_table (field_varchar) VALUES (?)"
    );
    stmt.validate_no_error();

    // Insert one value
    auto result = net->execute_statement(stmt.value, make_value_vector("value0"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(result.value.complete());
    BOOST_TEST(result.value.fields().empty());

    // Insert another one
    result = net->execute_statement(stmt.value, make_value_vector("value1"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(result.value.complete());
    BOOST_TEST(result.value.fields().empty());

    // Validate that the insertions took place
    BOOST_TEST(this->get_table_size("inserts_table") == 2);

    // Close it
    auto close_result = net->close_statement(stmt.value);
    close_result.validate_no_error();
}

BOOST_MYSQL_NETWORK_TEST(update_with_parameters_multiple_executions, network_fixture, network_gen)
{
    this->connect();
    this->start_transaction();
    auto* net = sample.net;

    // Prepare a statement
    auto stmt = net->prepare_statement(
        this->conn,
        "UPDATE updates_table SET field_int = ? WHERE field_varchar = ?"
    );
    stmt.validate_no_error();

    // Set field_int to something
    auto result = net->execute_statement(stmt.value, make_value_vector(200, "f0"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(result.value.complete());
    BOOST_TEST(result.value.fields().empty());

    // Verify that took effect
    BOOST_TEST(get_updates_table_value(this->conn) == value(std::int32_t(200)));

    // Set field_int to something different
    result = net->execute_statement(stmt.value, make_value_vector(250, "f0"));
    result.validate_no_error();
    BOOST_TEST(result.value.valid());
    BOOST_TEST(result.value.complete());
    BOOST_TEST(result.value.fields().empty());

    // Verify that took effect
    BOOST_TEST(get_updates_table_value(this->conn) == value(std::int32_t(250)));

    // Close the statement
    auto close_result = net->close_statement(stmt.value);
    close_result.validate_no_error();
}

BOOST_MYSQL_NETWORK_TEST(multiple_statements, network_fixture, network_gen)
{
    this->connect();
    this->start_transaction();
    auto* net = sample.net;

    // Prepare an update
    auto stmt_update = net->prepare_statement(
        this->conn,
        "UPDATE updates_table SET field_int = ? WHERE field_varchar = ?"
    );
    stmt_update.validate_no_error();

    // Prepare a select
    auto stmt_select = net->prepare_statement(
        this->conn,
        "SELECT field_int FROM updates_table WHERE field_varchar = ?"
    );
    stmt_select.validate_no_error();

    // They have different IDs
    BOOST_TEST(stmt_update.value.id() != stmt_select.value.id());

    // Execute update
    auto update_result = net->execute_statement(stmt_update.value, make_value_vector(210, "f0"));
    update_result.validate_no_error();
    BOOST_TEST(update_result.value.complete());

    // Execute select
    auto select_result = net->execute_statement(stmt_select.value, make_value_vector("f0"));
    select_result.validate_no_error();
    auto rows = net->read_all(select_result.value);
    rows.validate_no_error();
    BOOST_TEST(rows.value.size() == 1);
    BOOST_TEST(rows.value[0] == makerow(210));

    // Execute update again
    update_result = net->execute_statement(stmt_update.value, make_value_vector(220, "f0"));
    update_result.validate_no_error();
    BOOST_TEST(update_result.value.complete());

    // Update no longer needed, close it
    auto close_result = net->close_statement(stmt_update.value);
    close_result.validate_no_error();

    // Execute select again
    select_result = net->execute_statement(stmt_select.value, make_value_vector("f0"));
    select_result.validate_no_error();
    rows = net->read_all(select_result.value);
    rows.validate_no_error();
    BOOST_TEST(rows.value.size() == 1);
    BOOST_TEST(rows.value[0] == makerow(220));

    // Close select
    close_result = net->close_statement(stmt_select.value);
    close_result.validate_no_error();
}

BOOST_MYSQL_NETWORK_TEST(insert_with_null_values, network_fixture, network_gen)
{
    this->connect();
    this->start_transaction();
    auto* net = sample.net;

    // Statement to perform the updates
    auto stmt = net->prepare_statement(
        this->conn,
        "UPDATE updates_table SET field_int = ? WHERE field_varchar = 'fnull'"
    );
    stmt.validate_no_error();

    // Set the value we will be updating to something non-NULL
    auto result = net->execute_statement(stmt.value, make_value_vector(42));
    result.validate_no_error();

    // Verify it took effect
    BOOST_TEST_REQUIRE((
        get_updates_table_value(this->conn, "fnull") ==
            value(std::int32_t(42))));

    // Update the value to NULL
    result = net->execute_statement(stmt.value, make_value_vector(nullptr));
    result.validate_no_error();

    // Verify it took effect
    BOOST_TEST_REQUIRE((
        get_updates_table_value(this->conn, "fnull") == value(nullptr)));

    // Close statement
    auto close_result = net->close_statement(stmt.value);
    close_result.validate_no_error();
}

BOOST_AUTO_TEST_SUITE_END() // test_prepared_statement_lifecycle
