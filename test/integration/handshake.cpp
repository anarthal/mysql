//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/mysql/connection_params.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include "boost/mysql/errc.hpp"
#include "er_network_variant.hpp"
#include "get_endpoint.hpp"
#include "integration_test_common.hpp"
#include "streams.hpp"
#include "test_common.hpp"
#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/test/unit_test_suite.hpp>

// Tests containing with label 'sha256' require SHA256 functionality.
// This is used by the build script to exclude these tests on databases
// that do not support this functionality

using namespace boost::mysql::test;

using boost::mysql::ssl_mode;
using boost::mysql::errc;
using boost::mysql::error_code;
using boost::mysql::tcp_ssl_connection;
using boost::mysql::connection_params;

BOOST_AUTO_TEST_SUITE(test_handshake)

struct handshake_fixture : network_fixture
{
    void setup_and_physical_connect(network_variant* net)
    {
        setup(net);
        conn->physical_connect(er_endpoint::localhost).validate_no_error();
    }

    void do_handshake_ok()
    {
        conn->handshake(params).validate_no_error();
        BOOST_TEST(conn->uses_ssl() == var->supports_ssl());
    }

    void do_handshake_ok_ssl()
    {
        params.set_ssl(ssl_mode::require);
        conn->handshake(params).validate_no_error();
        BOOST_TEST(conn->uses_ssl());
    }

    void do_handshake_ok_nossl()
    {
        params.set_ssl(ssl_mode::disable);
        conn->handshake(params).validate_no_error();
        BOOST_TEST(!conn->uses_ssl());
    }
};

// mysql_native_password
BOOST_AUTO_TEST_SUITE(mysql_native_password)

BOOST_MYSQL_NETWORK_TEST(regular_user, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("mysqlnp_user", "mysqlnp_password");
    do_handshake_ok();
}

BOOST_MYSQL_NETWORK_TEST(empty_password, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("mysqlnp_empty_password_user", "");
    do_handshake_ok();
}

BOOST_MYSQL_NETWORK_TEST(bad_password, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("mysqlnp_user", "bad_password");
    conn->handshake(params).validate_error(
        errc::access_denied_error, {"access denied", "mysqlnp_user"});
}

BOOST_AUTO_TEST_SUITE_END() // mysql_native_password

// caching_sha2_password
BOOST_TEST_DECORATOR(*boost::unit_test::label("sha256"))
BOOST_AUTO_TEST_SUITE(caching_sha2_password)

struct caching_sha2_fixture : handshake_fixture
{
    void load_sha256_cache(boost::string_view user, boost::string_view password)
    {
        tcp_ssl_connection conn (ctx, ssl_ctx);
        conn.connect(
            get_endpoint<tcp_socket>(er_endpoint::localhost),
            connection_params(user, password)
        );
        conn.close();
    }

    void clear_sha256_cache()
    {
        tcp_ssl_connection conn (ctx, ssl_ctx);
        conn.connect(
            get_endpoint<tcp_socket>(er_endpoint::localhost),
            connection_params("root", "")
        );
        conn.query("FLUSH PRIVILEGES");
        conn.close();
    }
};

BOOST_MYSQL_NETWORK_TEST_SSL(ssl_on_cache_hit, caching_sha2_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "csha2p_password");
    load_sha256_cache("csha2p_user", "csha2p_password");
    do_handshake_ok_ssl();
}

BOOST_MYSQL_NETWORK_TEST(ssl_off_cache_hit, caching_sha2_fixture)
{
    // As we are sending password hashed, it is OK to not have SSL for this
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "csha2p_password");
    load_sha256_cache("csha2p_user", "csha2p_password");
    do_handshake_ok_nossl();
}

BOOST_MYSQL_NETWORK_TEST_SSL(ssl_on_cache_miss, caching_sha2_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "csha2p_password");
    clear_sha256_cache();
    do_handshake_ok_ssl();
}


BOOST_MYSQL_NETWORK_TEST(ssl_off_cache_miss, caching_sha2_fixture)
{
    // A cache miss would force us send a plaintext password over
    // a non-TLS connection, so we fail
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "csha2p_password");
    clear_sha256_cache();
    params.set_ssl(ssl_mode::disable);
    conn->handshake(params).validate_error(errc::auth_plugin_requires_ssl, {});
}

BOOST_MYSQL_NETWORK_TEST_SSL(empty_password_ssl_on_cache_hit, caching_sha2_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_empty_password_user", "");
    load_sha256_cache("csha2p_empty_password_user", "");
    do_handshake_ok_ssl();
}

BOOST_MYSQL_NETWORK_TEST(empty_password_ssl_off_cache_hit, caching_sha2_fixture)
{
    // Empty passwords are allowed over non-TLS connections
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_empty_password_user", "");
    load_sha256_cache("csha2p_empty_password_user", "");
    do_handshake_ok_nossl();
}

BOOST_MYSQL_NETWORK_TEST_SSL(empty_password_ssl_on_cache_miss, caching_sha2_fixture)
{
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_empty_password_user", "");
    clear_sha256_cache();
    do_handshake_ok_ssl();
}

BOOST_MYSQL_NETWORK_TEST(empty_password_ssl_off_cache_miss, caching_sha2_fixture)
{
    // Empty passwords are allowed over non-TLS connections
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_empty_password_user", "");
    clear_sha256_cache();
    do_handshake_ok_nossl();
}

BOOST_MYSQL_NETWORK_TEST_SSL(bad_password_ssl_on_cache_hit, caching_sha2_fixture)
{
    // Note: test over non-TLS would return "ssl required"
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "bad_password");
    load_sha256_cache("csha2p_user", "csha2p_password");
    conn->handshake(params).validate_error(
        errc::access_denied_error, {"access denied", "csha2p_user"});
}

BOOST_MYSQL_NETWORK_TEST_SSL(bad_password_ssl_on_cache_miss, caching_sha2_fixture)
{
    // Note: test over non-TLS would return "ssl required"
    setup_and_physical_connect(sample.net);
    set_credentials("csha2p_user", "bad_password");
    clear_sha256_cache();
    conn->handshake(params).validate_error(
        errc::access_denied_error, {"access denied", "csha2p_user"});
}

BOOST_AUTO_TEST_SUITE_END() // caching_sha2_password

// SSL certificate validation
BOOST_AUTO_TEST_SUITE(ssl_certificate_validation)

// The CA file that signed the server's certificate
constexpr const char CA_PEM [] = R"%(-----BEGIN CERTIFICATE-----
MIIDZzCCAk+gAwIBAgIUWznm2UoxXw3j7HCcp9PpiayTvFQwDQYJKoZIhvcNAQEL
BQAwQjELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxDjAMBgNVBAoM
BW15c3FsMQ4wDAYDVQQDDAVteXNxbDAgFw0yMDA0MDQxNDMwMjNaGA8zMDE5MDgw
NjE0MzAyM1owQjELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxDjAM
BgNVBAoMBW15c3FsMQ4wDAYDVQQDDAVteXNxbDCCASIwDQYJKoZIhvcNAQEBBQAD
ggEPADCCAQoCggEBAN0WYdvsDb+a0TxOGPejcwZT0zvTrf921mmDUlrLN1Z0hJ/S
ydgQCSD7Q+6za4lTFZCXcvs52xvvS2gfC0yXyYLCT/jA4RQRxuF+/+w1gDWEbGk0
KzEpsBuKrEIvEaVdoS78SxInnW/aegshdrRRocp4JQ6KHsZgkLTxSwPfYSUmMUo0
cRO0Q/ak3VK8NP13A6ZFvZjrBxjS3cSw9HqilgADcyj1D4EokvfI1C9LrgwgLlZC
XVkjjBqqoMXGGlnXOEK+pm8bU68HM/QvMBkb1Amo8pioNaaYgqJUCP0Ch0iu1nUU
HtsWt6emXv0jANgIW0oga7xcT4MDGN/M+IRWLTECAwEAAaNTMFEwHQYDVR0OBBYE
FNxhaGwf5ePPhzK7yOAKD3VF6wm2MB8GA1UdIwQYMBaAFNxhaGwf5ePPhzK7yOAK
D3VF6wm2MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAAoeJCAX
IDCFoAaZoQ1niI6Ac/cds8G8It0UCcFGSg+HrZ0YujJxWIruRCUG60Q2OAbEvn0+
uRpTm+4tV1Wt92WFeuRyqkomozx0g4CyfsxGX/x8mLhKPFK/7K9iTXM4/t+xQC4f
J+iRmPVsMKQ8YsHYiWVhlOMH9XJQiqERCB2kOKJCH6xkaF2k0GbM2sGgbS7Z6lrd
fsFTOIVx0VxLVsZnWX3byE9ghnDR5jn18u30Cpb/R/ShxNUGIHqRa4DkM5la6uZX
W1fpSW11JBSUv4WnOO0C2rlIu7UJWOROqZZ0OsybPRGGwagcyff2qVRuI2XFvAMk
OzBrmpfHEhF6NDU=
-----END CERTIFICATE-----
)%";

BOOST_MYSQL_NETWORK_TEST_SSL(certificate_valid, handshake_fixture)
{
    // Context changes need to be before setup
    ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx.add_certificate_authority(boost::asio::buffer(CA_PEM));
    setup_and_physical_connect(sample.net);
    do_handshake_ok_ssl();
}

BOOST_MYSQL_NETWORK_TEST_SSL(certificate_invalid, handshake_fixture)
{
    ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    setup_and_physical_connect(sample.net);
    conn->handshake(params).validate_any_error({ "certificate verify failed" });
}

BOOST_MYSQL_NETWORK_TEST_SSL(custom_certificate_verification_failed, handshake_fixture)
{
    ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx.add_certificate_authority(boost::asio::buffer(CA_PEM));
    ssl_ctx.set_verify_callback(boost::asio::ssl::host_name_verification("host.name"));
    setup_and_physical_connect(sample.net);
    conn->handshake(params).validate_any_error({ "certificate verify failed" });
}

BOOST_MYSQL_NETWORK_TEST_SSL(custom_certificate_verification_ok, handshake_fixture)
{
    ssl_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_ctx.add_certificate_authority(boost::asio::buffer(CA_PEM));
    ssl_ctx.set_verify_callback(boost::asio::ssl::host_name_verification("mysql"));
    setup_and_physical_connect(sample.net);
    do_handshake_ok_ssl();
}

BOOST_AUTO_TEST_SUITE_END() // ssl_certificate_validation

// Other handshake tests
BOOST_MYSQL_NETWORK_TEST(no_database, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    params.set_database("");
    do_handshake_ok();
}

BOOST_MYSQL_NETWORK_TEST(bad_database, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    params.set_database("bad_database");
    conn->handshake(params).validate_error(
        errc::dbaccess_denied_error, {"database", "bad_database"});
}

BOOST_TEST_DECORATOR(*boost::unit_test::label("sha256"))
BOOST_MYSQL_NETWORK_TEST_SSL(unknown_auth_plugin, handshake_fixture)
{
    // Note: sha256_password is not supported, so it's an unknown plugin to us
    setup_and_physical_connect(sample.net);
    set_credentials("sha2p_user", "sha2p_password");
    conn->handshake(params).validate_error(errc::unknown_auth_plugin, {});
}

BOOST_MYSQL_NETWORK_TEST_SSL(bad_user, handshake_fixture)
{
    // unreliable without SSL. If the default plugin requires SSL
    // (like SHA256), this would fail with 'ssl required'
    setup_and_physical_connect(sample.net);
    set_credentials("non_existing_user", "bad_password");
    conn->handshake(params).validate_any_error(); // may be access denied or unknown auth plugin
}

BOOST_MYSQL_NETWORK_TEST(ssl_disable, handshake_fixture)
{
    // Both SSL and non-SSL streams will act as non-SSL streams
    setup_and_physical_connect(sample.net);
    params.set_ssl(ssl_mode::disable);
    conn->handshake(params).validate_no_error();
    BOOST_TEST(!conn->uses_ssl());
}

BOOST_MYSQL_NETWORK_TEST_NOSSL(ssl_enable_nonssl_streams, handshake_fixture)
{
    // Ignored by non-ssl streams
    setup_and_physical_connect(sample.net);
    params.set_ssl(ssl_mode::enable);
    conn->handshake(params).validate_no_error();
    BOOST_TEST(!conn->uses_ssl());
}

BOOST_MYSQL_NETWORK_TEST_SSL(ssl_enable_ssl_streams, handshake_fixture)
{
    // In all our CI systems, our servers support SSL, so
    // ssl_mode::enable will do the same as ssl_mode::require.
    // We test for this fact.
    setup_and_physical_connect(sample.net);
    params.set_ssl(ssl_mode::enable);
    conn->handshake(params).validate_no_error();
    BOOST_TEST(conn->uses_ssl());
}

BOOST_MYSQL_NETWORK_TEST_NOSSL(ssl_require_nonssl_streams, handshake_fixture)
{
    // Ignored by non-ssl streams
    setup_and_physical_connect(sample.net);
    params.set_ssl(ssl_mode::require);
    conn->handshake(params).validate_no_error();
    BOOST_TEST(!conn->uses_ssl());
}

BOOST_MYSQL_NETWORK_TEST_SSL(ssl_require_ssl_streams, handshake_fixture)
{
    setup_and_physical_connect(sample.net);
    params.set_ssl(ssl_mode::require);
    conn->handshake(params).validate_no_error();
    BOOST_TEST(conn->uses_ssl());
}

BOOST_AUTO_TEST_SUITE_END() // test_handshake
