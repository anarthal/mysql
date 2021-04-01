//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP

#include "boost/mysql/detail/protocol/channel.hpp"
#include "boost/mysql/error.hpp"
#include <boost/mysql/detail/network_algorithms/quit_connection.hpp>
#include <boost/asio/post.hpp>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

template <typename Stream>
void shutdown_ssl_impl(
    Stream& s,
    std::true_type // supports ssl
)
{
    error_code ignored;
    s.shutdown(ignored);
}

template <typename Stream>
void shutdown_ssl_impl(
    Stream&,
    std::false_type // supports ssl
)
{
}

template <typename Stream>
void shutdown_ssl(Stream& s) { shutdown_ssl_impl(s, is_ssl_stream<Stream>()); }

template<class Stream, bool is_ssl_stream>
struct shutdown_ssl_op;

template <class Stream>
struct shutdown_ssl_op<Stream, true> : boost::asio::coroutine
{
    Stream& s_;

    shutdown_ssl_op(Stream& s) noexcept : s_(s) {} 

    template<class Self>
    void operator()(
        Self& self,
        error_code err = {}
    )
    {
        BOOST_ASIO_CORO_REENTER(*this)
        {
            BOOST_ASIO_CORO_YIELD s_.async_shutdown(std::move(self));
            self.complete(err);
        }
    }
};

template <class Stream>
struct shutdown_ssl_op<Stream, false>
{
    shutdown_ssl_op(Stream&) noexcept {}

    template<class Self>
    void operator()(
        Self& self,
        error_code err = {}
    )
    {
        self.complete(err);
    }
};


template <class Stream, class CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(
    CompletionToken,
    void(boost::mysql::error_code)
)
async_shutdown_ssl(
    Stream& s,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<
        CompletionToken,
        void(error_code)
    >(
        shutdown_ssl_op<Stream, is_ssl_stream<Stream>::value>(s),
        token,
        s
    );
}



template<class SocketStream>
struct close_connection_op : boost::asio::coroutine
{
    channel<SocketStream>& chan_;
    error_info& output_info_;
    error_code err_to_return_;

    close_connection_op(channel<SocketStream>& chan, error_info& output_info) :
        chan_(chan),
        output_info_(output_info)
    {
    }

    template<class Self>
    void operator()(
        Self& self,
        error_code err = {}
    )
    {
        error_code close_err;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            if (!chan_.lowest_layer().is_open())
            {
                BOOST_ASIO_CORO_YIELD boost::asio::post(std::move(self));
                self.complete(error_code());
                BOOST_ASIO_CORO_YIELD break;
            }

            // We call close regardless of the quit outcome
            // There are no async versions of shutdown or close
            BOOST_ASIO_CORO_YIELD async_quit_connection(
                chan_,
                std::move(self),
                output_info_
            );
            err_to_return_ = err;

            // SSL shutdown error ignored, as MySQL doesn't always gracefully
            // close SSL connections
            BOOST_ASIO_CORO_YIELD async_shutdown_ssl(chan_.next_layer(), std::move(self));

            close_err = chan_.close();
            self.complete(err_to_return_ ? err_to_return_ : close_err);
        }
    }
};

}
}
}

template <class SocketStream>
void boost::mysql::detail::close_connection(
    channel<SocketStream>& chan,
    error_code& code,
    error_info& info
)
{
    // Close = quit + close stream. We close the stream regardless of the quit failing or not
    if (chan.lowest_layer().is_open())
    {
        // MySQL quit notification
        quit_connection(chan, code, info);

        // SSL shutdown. Result ignored as MySQL does not always perform
        // graceful SSL shutdowns
        shutdown_ssl(chan.next_layer());
        
        auto err = chan.close();
        if (!code)
        {
            code = err;
        }
    }
}

template <class SocketStream, class CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(
    CompletionToken,
    void(boost::mysql::error_code)
)
boost::mysql::detail::async_close_connection(
    channel<SocketStream>& chan,
    CompletionToken&& token,
    error_info& info
)
{
    return boost::asio::async_compose<
        CompletionToken,
        void(boost::mysql::error_code)
    >(close_connection_op<SocketStream>{chan, info}, token, chan);
}


#endif /* INCLUDE_BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_CLOSE_CONNECTION_HPP_ */
