//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../all_network_functions.hpp"

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

using namespace boost::mysql::test;

BOOST_MYSQL_NETFNS_INSTANTIATE(unix_ssl_socket)

#endif
