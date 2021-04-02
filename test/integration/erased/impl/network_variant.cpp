//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../network_variant.hpp"
#include <algorithm>
#include <functional>
#include <iterator>

using namespace boost::mysql::test;

static std::vector<network_variant*> make_all_variants()
{
    std::vector<network_variant*> res;
    add_sync_errc(res);
    return res;
}

static std::vector<network_variant*> make_ssl_variants()
{
    auto all = make_all_variants();
    std::vector<network_variant*> res;
    std::copy_if(
        all.begin(), 
        all.end(), 
        std::back_inserter(res), 
        [](network_variant* v) { return v->supports_ssl(); }
    );
    return res;
}

static std::vector<network_variant*> make_non_ssl_variants()
{
    auto all = make_all_variants();
    std::vector<network_variant*> res;
    std::copy_if(
        all.begin(), 
        all.end(), 
        std::back_inserter(res), 
        [](network_variant* v) { return !v->supports_ssl(); }
    );
    return res;
}

const std::vector<network_variant*>& boost::mysql::test::all_variants()
{
    static auto res = make_all_variants();
    return res;
}

const std::vector<network_variant*>& boost::mysql::test::ssl_variants()
{
    static auto res = make_ssl_variants();
    return res;
}

const std::vector<network_variant*>& boost::mysql::test::non_ssl_variants()
{
    static auto res = make_non_ssl_variants();
    return res;
}