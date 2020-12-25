//
// Copyright (c) 2019-2020 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_CONNECTION_PARAMS_HPP
#define BOOST_MYSQL_CONNECTION_PARAMS_HPP

#include <boost/utility/string_view.hpp>
#include "boost/mysql/collation.hpp"

namespace boost {
namespace mysql {

/// Determines whether to use TLS for the connection or not.
enum class ssl_mode
{
    /** Never use TLS
     */
    disable,

    /** Use TLS if the server supports it, fall back to non-encrypted connection if it does not.
     */
    enable,

    /** Always use TLS; abort the connection if the server does not support it.
     */
    require
};

/**
 * \brief Connection options regarding TLS.
 * \details At the moment, contains only the [reflink ssl_mode], which
 * indicates whether to use TLS on the connection or not.
 */
class ssl_options
{
    ssl_mode mode_;
public:
    /**
     * \brief Default and initialization constructor.
     * \details By default, SSL is enabled for the connection
     * if the server supports it (ssl_mode::enable).
     * See [reflink ssl_mode].
     */
    explicit ssl_options(ssl_mode mode=ssl_mode::enable) noexcept:
        mode_(mode) {}

    /// Retrieves the TLS mode to be used for the connection.
    ssl_mode mode() const noexcept { return mode_; }
};


/**
 * \brief Parameters defining how to perform the handshake
 * with a MySQL server. See [link mysql.connparams this section]
 * for more information on each parameter.
 */
class connection_params
{
    boost::string_view username_;
    boost::string_view password_;
    boost::string_view database_;
    collation connection_collation_;
    ssl_options ssl_;
public:
    /**
     * \brief Initializing constructor
     * \param username User name to authenticate as.
     * \param password Password for that username, possibly empty.
     * \param db Database name to use, or empty string for no database (this is the default).
     * \param connection_col [reflink2 collation Collation] to use for the connection.
     * Impacts how text queries and prepared statements are interpreted. Defaults to utf8_general_ci.
     * \param opts The [reflink2 ssl_options TLS options] to use with this connection.
     */
    connection_params(
        boost::string_view username,
        boost::string_view password,
        boost::string_view db = "",
        collation connection_col = collation::utf8_general_ci,
        const ssl_options& opts = ssl_options()
    ) :
        username_(username),
        password_(password),
        database_(db),
        connection_collation_(connection_col),
        ssl_(opts)
    {
    }

    /// Retrieves the username.
    boost::string_view username() const noexcept { return username_; }

    /// Sets the username.
    void set_username(boost::string_view value) noexcept { username_ = value; }

    /// Retrieves the password.
    boost::string_view password() const noexcept { return password_; }

    /// Sets the password
    void set_password(boost::string_view value) noexcept { password_ = value; }

    /// Retrieves the database.
    boost::string_view database() const noexcept { return database_; }

    /// Sets the database
    void set_database(boost::string_view value) noexcept { database_ = value; }

    /// Retrieves the connection collation.
    collation connection_collation() const noexcept { return connection_collation_; }

    /// Sets the connection collation
    void set_connection_collation(collation value) noexcept { connection_collation_ = value; }

    /// Retrieves SSL options
    const ssl_options& ssl() const noexcept { return ssl_; }

    /// Sets SSL options
    void set_ssl(const ssl_options& value) noexcept { ssl_ = value; }
};

} // mysql
} // boost



#endif /* INCLUDE_BOOST_MYSQL_CONNECTION_PARAMS_HPP_ */
