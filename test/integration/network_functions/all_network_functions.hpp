//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_ALL_NETWORK_FUNCTIONS_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_FUNCTIONS_ALL_NETWORK_FUNCTIONS_HPP

#include "network_functions.hpp"
#include "sync.hpp"
#include "async_callback.hpp"
#include "async_coroutine.hpp"
#include "async_coroutinecpp20.hpp"
#include "async_future.hpp"
#include "../stream_list.hpp"

namespace boost {
namespace mysql {
namespace test {

template <class NetworkFunctionsImpl>
NetworkFunctionsImpl* network_functions_singleton()
{
    static NetworkFunctionsImpl res;
    return &res;
}

template <class Stream>
std::vector<network_functions<Stream>*> make_all_network_functions()
{
    return {
        network_functions_singleton<sync_errc<Stream>>(),
        network_functions_singleton<sync_exc<Stream>>(),
        network_functions_singleton<async_callback_errinfo<Stream>>(),
        network_functions_singleton<async_callback_noerrinfo<Stream>>(),
        network_functions_singleton<async_coroutine_errinfo<Stream>>(),
        network_functions_singleton<async_coroutine_noerrinfo<Stream>>(),
        network_functions_singleton<async_coroutinecpp20_errinfo<Stream>>(),
        network_functions_singleton<async_coroutinecpp20_noerrinfo<Stream>>(),
        network_functions_singleton<async_future_errinfo<Stream>>(),
        network_functions_singleton<async_future_noerrinfo<Stream>>(),
    };
}

template <>
inline std::vector<network_functions<tcp_ssl_future_socket>*> make_all_network_functions()
{
    return {
        network_functions_singleton<async_future_errinfo<tcp_ssl_future_socket>>(),
        network_functions_singleton<async_future_noerrinfo<tcp_ssl_future_socket>>(),
    };
}


template <class Stream>
const std::vector<network_functions<Stream>*>& all_network_functions()
{
    static auto res = make_all_network_functions<Stream>();
    return res;
}

} // test
} // mysql
} // boost

#define BOOST_MYSQL_NETFNS_INSTANTIATE(Stream) \
    template class boost::mysql::test::sync_errc<Stream>; \
    template class boost::mysql::test::sync_exc<Stream>; \
    template class boost::mysql::test::async_callback_errinfo<Stream>; \
    template class boost::mysql::test::async_callback_noerrinfo<Stream>;  \
    template class boost::mysql::test::async_coroutine_errinfo<Stream>; \
    template class boost::mysql::test::async_coroutine_noerrinfo<Stream>; \
    template class boost::mysql::test::async_coroutinecpp20_errinfo<Stream>; \
    template class boost::mysql::test::async_coroutinecpp20_noerrinfo<Stream>; \
    template class boost::mysql::test::async_future_errinfo<Stream>; \
    template class boost::mysql::test::async_future_noerrinfo<Stream>; 



#endif /* TEST_INTEGRATION_NETWORK_FUNCTIONS_HPP_ */
