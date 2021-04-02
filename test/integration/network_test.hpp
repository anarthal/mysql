//
// Copyright (c) 2019-2021 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_TEST_INTEGRATION_NETWORK_TEST_HPP
#define BOOST_MYSQL_TEST_INTEGRATION_NETWORK_TEST_HPP

#include <boost/test/unit_test.hpp>

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
template <class DataGen>
using data_gen_sample_type = typename std::decay<decltype(
    std::declval<typename std::decay<DataGen>::type>()()[0]
)>::type;

inline boost::unit_test::test_suite* create_test_suite(
    boost::unit_test::const_string tc_name,
    boost::unit_test::const_string tc_file,
    std::size_t tc_line
)
{
    // Create a test suite with the name of the test
    auto* suite = new boost::unit_test::test_suite(tc_name, tc_file, tc_line);
    boost::unit_test::framework::current_auto_test_suite().add(suite);

    // Add decorators
    auto& collector = boost::unit_test::decorator::collector_t::instance();
    collector.store_in(*suite);
    collector.reset();

    return suite;
}

// Inspired in how Boost.Test auto-registers unit tests.
// BOOST_MYSQL_NETWORK_TEST defines a static variable of this
// type, which takes care of test registration.
template <class Testcase>
struct network_test_registrar
{
    template <class DataGen>
    network_test_registrar(
        boost::unit_test::const_string tc_name,
        boost::unit_test::const_string tc_file,
        std::size_t tc_line,
        const DataGen& datagen
    )
    {
        // Create suite
        auto* suite = create_test_suite(tc_name, tc_file, tc_line);

        // Create a test for each sample
        for (const auto& sample : datagen())
        {
            std::string test_name = stringize(sample);
            auto* test = boost::unit_test::make_test_case(
                [sample] {
                    Testcase tc_struct;
                    tc_struct.test_method(sample);
                },
                test_name,
                tc_file,
                tc_line
            );
            sample.set_test_attributes(*test);
            suite->add(test);
        }
    }
};

} // test
} // mysql
} // boost

#define BOOST_MYSQL_NETWORK_TEST_EX(name, fixture, sample_generator) \
    struct name : public fixture \
    { \
        void test_method(const data_gen_sample_type<decltype(sample_generator)>&); \
    }; \
    static ::boost::mysql::test::network_test_registrar<name> \
        name##_registrar BOOST_ATTRIBUTE_UNUSED ( \
        #name, __FILE__, __LINE__, sample_generator); \
    void name::test_method( \
        const data_gen_sample_type<decltype(sample_generator)>& sample \
    )

#define BOOST_MYSQL_NETWORK_TEST(name, fixture) \
    BOOST_MYSQL_NETWORK_TEST_EX(name, fixture, all_variants_gen())


#endif
