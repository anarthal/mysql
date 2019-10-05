/*
 * deserialization.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: ruben
 */

#include "serialization_test_common.hpp"

using namespace testing;
using namespace std;
using namespace mysql;
using namespace mysql::detail;

namespace
{

struct DeserializeErrorParams
{
	shared_ptr<TypeErasedValue> value;
	vector<uint8_t> buffer;
	string test_name;
	Error expected_error;

	template <typename T>
	DeserializeErrorParams create(vector<uint8_t>&& buffer, string&& test_name, Error err = Error::incomplete_message)
	{
		return DeserializeErrorParams {
			make_shared<TypeErasedValueImpl<T>>(T{}),
			move(buffer),
			move(test_name),
			err
		};
	}
};

ostream& operator<<(ostream& os, const DeserializeErrorParams& params)
{
	return os << params.value->get_type_name() << " - " << params.test_name;
}

// Special error conditions
struct DeserializeErrorTest : testing::TestWithParam<DeserializeErrorParams>
{
};

TEST_P(DeserializeErrorTest, Deserialize_ErrorCondition_ReturnsErrorCode)
{
	auto first = GetParam().buffer.data();
	auto last = GetParam().buffer.data() + GetParam().buffer.size();
	DeserializationContext ctx (first, last, 0);
	auto value = GetParam().value->default_construct();
	auto err = value->deserialize(ctx);
	EXPECT_EQ(err, GetParam().expected_error);
}

// Definitions for the parameterized tests
string string_250 (250, 'a');
string string_251 (251, 'a');
string string_ffff (0xffff, 'a');
string string_10000 (0x10000, 'a');

enum class EnumInt1 : uint8_t
{
	value0 = 0,
	value1 = 3,
	value2 = 0xff
};

enum class EnumInt2 : uint16_t
{
	value0 = 0,
	value1 = 3,
	value2 = 0xfeff
};

enum class EnumInt4 : uint32_t
{
	value0 = 0,
	value1 = 3,
	value2 = 0xfcfdfeff
};

INSTANTIATE_TEST_SUITE_P(UnsignedFixedSizeInts, FullSerializationTest, ::testing::Values(
	SerializeParams(int1{0xff}, {0xff}),
	SerializeParams(int2{0xfeff}, {0xff, 0xfe}),
	SerializeParams(int3{0xfdfeff}, {0xff, 0xfe, 0xfd}),
	SerializeParams(int4{0xfcfdfeff}, {0xff, 0xfe, 0xfd, 0xfc}),
	SerializeParams(int6{0xfafbfcfdfeff}, {0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa}),
	SerializeParams(int8{0xf8f9fafbfcfdfeff}, {0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8})
));

INSTANTIATE_TEST_SUITE_P(SignedFixedSizeInts, FullSerializationTest, ::testing::Values(
	SerializeParams(int1_signed{-1}, {0xff}, "Negative"),
	SerializeParams(int2_signed{-0x101}, {0xff, 0xfe}, "Negative"),
	SerializeParams(int4_signed{-0x3020101}, {0xff, 0xfe, 0xfd, 0xfc}, "Negative"),
	SerializeParams(int8_signed{-0x0706050403020101}, {0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8}, "Negative"),
	SerializeParams(int1_signed{0x01}, {0x01}, "Positive"),
	SerializeParams(int2_signed{0x0201}, {0x01, 0x02}, "Positive"),
	SerializeParams(int4_signed{0x04030201}, {0x01, 0x02, 0x03, 0x04}, "Positive"),
	SerializeParams(int8_signed{0x0807060504030201}, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}, "Positive")
));

INSTANTIATE_TEST_SUITE_P(LengthEncodedInt, FullSerializationTest, ::testing::Values(
	SerializeParams(int_lenenc{1}, {0x01}, "1 byte (regular value)"),
	SerializeParams(int_lenenc{250}, {0xfa}, "1 byte (max value)"),
	SerializeParams(int_lenenc{0xfeb7}, {0xfc, 0xb7, 0xfe}, "2 bytes (regular value)"),
	SerializeParams(int_lenenc{0xffff}, {0xfc, 0xff, 0xff}, "2 bytes (max value)"),
	SerializeParams(int_lenenc{0xa0feff}, {0xfd, 0xff, 0xfe, 0xa0}, "3 bytes (regular value)"),
	SerializeParams(int_lenenc{0xffffff}, {0xfd, 0xff, 0xff, 0xff}, "3 bytes (max value)"),
	SerializeParams(int_lenenc{0xf8f9fafbfcfdfeff},
			{0xfe, 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8}, "8 bytes (regular value)"),
	SerializeParams(int_lenenc{0xffffffffffffffff},
			{0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, "8 bytes (max value)")
));

INSTANTIATE_TEST_SUITE_P(FixedSizeString, FullSerializationTest, ::testing::Values(
	SerializeParams(string_fixed<4>{'a', 'b', 'd', 'e'}, {0x61, 0x62, 0x64, 0x65}, "Regular characters"),
	SerializeParams(string_fixed<3>{'\0', '\1', 'a'}, {0x00, 0x01, 0x61}, "Null characters"),
	SerializeParams(string_fixed<3>{char(0xc3), char(0xb1), 'a'}, {0xc3, 0xb1, 0x61}, "UTF-8 characters"),
	SerializeParams(string_fixed<1>{'a'}, {0x61}, "Size 1 string")
));

INSTANTIATE_TEST_SUITE_P(NullTerminatedString, FullSerializationTest, ::testing::Values(
	SerializeParams(string_null{"abc"}, {0x61, 0x62, 0x63, 0x00}, "Regular characters"),
	SerializeParams(string_null{"\xc3\xb1"}, {0xc3, 0xb1, 0x00}, "UTF-8 characters"),
	SerializeParams(string_null{""}, {0x00}, "Empty string")
));

INSTANTIATE_TEST_SUITE_P(LengthEncodedString, FullSerializationTest, ::testing::Values(
	SerializeParams(string_lenenc{""}, {0x00}, "Empty string"),
	SerializeParams(string_lenenc{"abc"}, {0x03, 0x61, 0x62, 0x63}, "1 byte size, regular characters"),
	SerializeParams(string_lenenc{string_view("a\0b", 3)}, {0x03, 0x61, 0x00, 0x62}, "1 byte size, null characters"),
	SerializeParams(string_lenenc{string_250}, concat({250}, vector<uint8_t>(250, 0x61)), "1 byte size, max"),
	SerializeParams(string_lenenc{string_251}, concat({0xfc, 251, 0}, vector<uint8_t>(251, 0x61)), "2 byte size, min"),
	SerializeParams(string_lenenc{string_ffff}, concat({0xfc, 0xff, 0xff}, vector<uint8_t>(0xffff, 0x61)), "2 byte size, max"),
	SerializeParams(string_lenenc{string_10000}, concat({0xfd, 0x00, 0x00, 0x01}, vector<uint8_t>(0x10000, 0x61)), "3 byte size, max")
));

INSTANTIATE_TEST_SUITE_P(EofString, SerializeDeserializeTest, ::testing::Values(
	SerializeParams(string_eof{"abc"}, {0x61, 0x62, 0x63}, "Regular characters"),
	SerializeParams(string_eof{string_view("a\0b", 3)}, {0x61, 0x00, 0x62}, "Null characters"),
	SerializeParams(string_eof{""}, {}, "Empty string")
));

INSTANTIATE_TEST_SUITE_P(Enums, FullSerializationTest, ::testing::Values(
	SerializeParams(EnumInt1::value1, {0x03}, "low value"),
	SerializeParams(EnumInt1::value2, {0xff}, "high value"),
	SerializeParams(EnumInt2::value1, {0x03, 0x00}, "low value"),
	SerializeParams(EnumInt2::value2, {0xff, 0xfe}, "high value"),
	SerializeParams(EnumInt4::value1, {0x03, 0x00, 0x00, 0x00}, "low value"),
	SerializeParams(EnumInt4::value2, {0xff, 0xfe, 0xfd, 0xfc}, "high value")
));

INSTANTIATE_TEST_SUITE_P(PacketHeader, FullSerializationTest, ::testing::Values(
	// packet header
	SerializeParams(msgs::packet_header{{3}, {0}}, {0x03, 0x00, 0x00, 0x00}, "small packet, seqnum==0"),
	SerializeParams(msgs::packet_header{{9}, {2}}, {0x09, 0x00, 0x00, 0x02}, "small packet, seqnum!=0"),
	SerializeParams(msgs::packet_header{{0xcacbcc}, {0xfa}}, {0xcc, 0xcb, 0xca, 0xfa}, "big packet, seqnum!=0"),
	SerializeParams(msgs::packet_header{{0xffffff}, {0xff}}, {0xff, 0xff, 0xff, 0xff}, "max packet, max seqnum")
));

INSTANTIATE_TEST_SUITE_P(OkPacket, DeserializeTest, ::testing::Values(
	SerializeParams(msgs::ok_packet{
		{4}, // affected rows
		{0}, // last insert ID
		{SERVER_STATUS_AUTOCOMMIT | SERVER_QUERY_NO_INDEX_USED}, // server status
		{0}, // warnings
		string_lenenc{"Rows matched: 5  Changed: 4  Warnings: 0"}
	}, {
		0x04, 0x00, 0x22, 0x00, 0x00, 0x00, 0x28, 0x52, 0x6f, 0x77, 0x73,
		0x20, 0x6d, 0x61, 0x74, 0x63, 0x68, 0x65, 0x64, 0x3a, 0x20, 0x35, 0x20, 0x20, 0x43, 0x68, 0x61,
		0x6e, 0x67, 0x65, 0x64, 0x3a, 0x20, 0x34, 0x20, 0x20, 0x57, 0x61, 0x72, 0x6e, 0x69, 0x6e, 0x67,
		0x73, 0x3a, 0x20, 0x30
	}, "successful UPDATE"),

	SerializeParams(msgs::ok_packet{
		{1}, // affected rows
		{6}, // last insert ID
		{SERVER_STATUS_AUTOCOMMIT}, // server status
		{0}, // warnings
		{}  // no message
	},{
		0x01, 0x06, 0x02, 0x00, 0x00, 0x00
	}, "successful INSERT")
));

INSTANTIATE_TEST_SUITE_P(ErrPacket, DeserializeTest, ::testing::Values(
	SerializeParams(msgs::err_packet{
		{1049}, // eror code
		{0x23}, // sql state marker
		{'4', '2', '0', '0', '0'}, // sql state
		{"Unknown database 'a'"} // err msg
	}, {
		0x19, 0x04, 0x23, 0x34, 0x32, 0x30, 0x30, 0x30, 0x55, 0x6e, 0x6b,
		0x6e, 0x6f, 0x77, 0x6e, 0x20, 0x64, 0x61, 0x74,
		0x61, 0x62, 0x61, 0x73, 0x65, 0x20, 0x27, 0x61, 0x27
	}, "Wrong USE database"),

	SerializeParams(msgs::err_packet{
		{1146}, // eror code
		{0x23}, // sql state marker
		{'4', '2', 'S', '0', '2'}, // sql state
		{"Table 'awesome.unknown' doesn't exist"} // err msg
	}, {
		0x7a, 0x04, 0x23, 0x34, 0x32, 0x53, 0x30, 0x32,
		0x54, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x27, 0x61,
		0x77, 0x65, 0x73, 0x6f, 0x6d, 0x65, 0x2e, 0x75,
		0x6e, 0x6b, 0x6e, 0x6f, 0x77, 0x6e, 0x27, 0x20,
		0x64, 0x6f, 0x65, 0x73, 0x6e, 0x27, 0x74, 0x20,
		0x65, 0x78, 0x69, 0x73, 0x74
	}, "Unknown table")
));


} // anon namespace