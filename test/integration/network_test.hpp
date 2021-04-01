//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_TEST_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_TEST_HPP

#include <boost/mp11/utility.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <memory>
#include "stream_list.hpp"

/**
 * Defines the required infrastructure for network tests.
 * This functionality is accessed via the BOOST_MYSQL_NETWORK_TEST
 * macro. Network tests combine data driven with templated test
 * functionality. They run the specified test across all samples
 * and stream types. Samples are passed in using a generator object
 * (runtime), while stream types are passed in as a MP11 list.
 *
 * We can't employ built-in Boost.Test functionality because:
 *   - Data driven + templated tests are not supported.
 *   - We need tests generated with different samples to have
 *     specific names and labels. This is used to filter out
 *     tests using SSL when running under Valgrind, to reduce run time.
 *
 * BOOST_MYSQL_NETWORK_TEST_EX is passed 4 arguments:
 *   - The test name. Should be a valid C++ identifier.
 *   - Fixture name. All network tests must have a single fixture.
 *     Only constructor/destructor fixtures are supported. The fixture
 *     should be a template with a single argument: the stream name.
 *     See network_fixture for the base class to use.
 *   - Data generator. The name of a class that specifies how to generate
 *     samples. It should have a static function with the following signature:
 *       template <class Stream>
 *       static SampleCollection generate(); // SampleCollection is a collection of samples.
 *     The generated samples may be different for each stream type.
 *     Samples should be streamable and have a
 *     void set_test_attributes(boost::unit_test::test_case&) const
 *     function, allowing the sample to add labels or other properties to the test.
 *   - Stream list. A mp11 list specifying which streams to test with.
 *
 * The helper macro BOOST_MYSQL_NETWORK_TEST runs tests across all
 * supported stream types.
 *
 * The most common generator is network_gen, to run
 * a single test over all network functions. See
 * network_functions.hpp for more info on network functions.
 *
 * After calling BOOST_MYSQL_NETWORK_TEST(_EX), you should define
 * a function body (like in BOOST_AUTO_TEST_CASE). The function
 * will be templated with a Stream type argument, and will have a
 * 'sample' parameter.
 */

namespace boost {
namespace mysql {
namespace test {

// The type of a sample generated by DataGenerator
// when running tests for stream type Stream.
template <class DataGenerator, class Stream>
using data_gen_sample_type = typename std::decay<
    decltype(*std::begin(DataGenerator:: template generate<Stream>()))
>::type;

template <class Self, class DataGenerator>
struct network_test_registrar_op
{
    Self& self;
    boost::unit_test::const_string tc_name;
    boost::unit_test::const_string tc_file;
    std::size_t tc_line;
    boost::unit_test::test_suite* suite;

    template <class Stream>
    void operator()(boost::mp11::mp_identity<Stream>) const
    {
        for (const auto& sample: DataGenerator:: template generate<Stream>())
        {
            self.template create_test<Stream>(sample, tc_file, tc_line, *suite);
        }
    }
};

// Inspired in how Boost.Test auto-registers unit tests.
// BOOST_MYSQL_NETWORK_TEST defines a static variable of this
// type, which takes care of test registration.
template <template <class> class Testcase, class TypeList>
struct network_test_registrar
{
    template <class DataGenerator>
    network_test_registrar(
        boost::unit_test::const_string tc_name,
        boost::unit_test::const_string tc_file,
        std::size_t tc_line,
        DataGenerator&&
    )
    {
        // Create a test suite with the name of the test
        auto* suite = new boost::unit_test::test_suite(tc_name, tc_file, tc_line);
        boost::unit_test::framework::current_auto_test_suite().add(suite);

        // Add decorators
        auto& collector = boost::unit_test::decorator::collector_t::instance();
        collector.store_in(*suite);
        collector.reset();

        // Create a test for each sample and each stream type
        network_test_registrar_op<network_test_registrar<Testcase, TypeList>, DataGenerator> op {
            *this, tc_name, tc_file, tc_line, suite
        };
        boost::mp11::mp_for_each<boost::mp11::mp_transform<boost::mp11::mp_identity, TypeList>>(op);
    }

    template <class Stream, class Sample>
    void create_test(
        const Sample& sample,
        boost::unit_test::const_string tc_file,
        std::size_t tc_line,
        boost::unit_test::test_suite& suite
    )
    {
        const char* stream_name = get_stream_name<Stream>();
        std::string test_name = stringize(stream_name, '_', sample);
        auto* test = boost::unit_test::make_test_case(
            [sample, test_name] {
                BOOST_TEST_CHECKPOINT(test_name << " with sample=" << sample << " setup");
                Testcase<Stream> tc_struct;
                BOOST_TEST_CHECKPOINT(test_name << " with sample=" << sample << " test start");
                tc_struct.test_method(sample);
                BOOST_TEST_CHECKPOINT(test_name << " with sample=" << sample << " teardown");
            },
            test_name,
            tc_file,
            tc_line
        );
        test->add_label(stream_name);
        if (supports_ssl<Stream>())
        {
            test->add_label("ssl");
        }
        sample.set_test_attributes(*test);
        suite.add(test);
    }
};

} // test
} // mysql
} // boost

#define BOOST_MYSQL_NETWORK_TEST_EX(name, fixture, data_gen, type_list) \
    template <class Stream> \
    struct name : public fixture<Stream> \
    { \
        void test_method(const data_gen_sample_type<data_gen, Stream>&); \
    }; \
    static ::boost::mysql::test::network_test_registrar<name, type_list> \
        name##_registrar BOOST_ATTRIBUTE_UNUSED ( \
        #name, __FILE__, __LINE__, data_gen()); \
    template <class Stream> \
    void name<Stream>::test_method( \
        const data_gen_sample_type<data_gen, Stream>& sample \
    )

#define BOOST_MYSQL_NETWORK_TEST(name, fixture, data_gen) \
    BOOST_MYSQL_NETWORK_TEST_EX(name, fixture, data_gen, ::boost::mysql::test::all_streams)


#endif
