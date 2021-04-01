//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_FUNCTIONS_IMPL_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_NETWORK_FUNCTIONS_IMPL_HPP

#include "../network_functions.hpp"
#include "../stream_list.hpp"

namespace boost {
namespace mysql {
namespace test {

template <class Stream> network_functions<Stream>* sync_errc_functions();
template <class Stream> network_functions<Stream>* sync_exc_functions();
template <class Stream> network_functions<Stream>* async_callback_errinfo_functions();
template <class Stream> network_functions<Stream>* async_callback_noerrinfo_functions();
template <class Stream> network_functions<Stream>* async_coroutine_errinfo_functions();
template <class Stream> network_functions<Stream>* async_coroutine_noerrinfo_functions();
template <class Stream> network_functions<Stream>* async_coroutinecpp20_errinfo_functions();
template <class Stream> network_functions<Stream>* async_coroutinecpp20_noerrinfo_functions();
template <class Stream> network_functions<Stream>* async_future_noerrinfo_functions();
template <class Stream> network_functions<Stream>* async_future_errinfo_functions();

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#define BOOST_MYSQL_INSTANTIATE_NETWORK_FUNCTIONS_UNIX(fun) \
    template boost::mysql::test::network_functions<boost::mysql::test::unix_socket>* \
        boost::mysql::test::fun<boost::mysql::test::unix_socket>(); \
    template boost::mysql::test::network_functions<boost::mysql::test::unix_ssl_socket>* \
        boost::mysql::test::fun<boost::mysql::test::unix_ssl_socket>();
#else
#define BOOST_MYSQL_INSTANTIATE_NETWORK_FUNCTIONS_UNIX(fun)
#endif

#define BOOST_MYSQL_INSTANTIATE_NETWORK_FUNCTIONS(fun) \
    template boost::mysql::test::network_functions<boost::mysql::test::tcp_socket>* \
    boost::mysql::test::fun<boost::mysql::test::tcp_socket>(); \
    template boost::mysql::test::network_functions<boost::mysql::test::tcp_ssl_socket>* \
    boost::mysql::test::fun<boost::mysql::test::tcp_ssl_socket>(); \
    BOOST_MYSQL_INSTANTIATE_NETWORK_FUNCTIONS_UNIX(fun)


}
}
}



#endif
