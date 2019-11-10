#ifndef INCLUDE_MYSQL_FIELD_TYPE_HPP_
#define INCLUDE_MYSQL_FIELD_TYPE_HPP_

#include <cstdint>

namespace mysql
{

enum class field_type : std::uint8_t
{
	decimal = 0x00,    // Apparently not sent
	tiny = 0x01,       // TINYINT
	short_ = 0x02,     // SMALLINT
	long_ = 0x03,      // INT
	float_ = 0x04,     // FLOAT
	double_ = 0x05,    // DOUBLE
	null = 0x06,       // Apparently not sent
	timestamp = 0x07,  // TIMESTAMP
	longlong = 0x08,   // BIGINT
	int24 = 0x09,      // MEDIUMINT
	date = 0x0a,       // DATE
	time = 0x0b,       // TIME
	datetime = 0x0c,   // DATETIME
	year = 0x0d,       // YEAR
	varchar = 0x0f,    // Apparently not sent
	bit = 0x10,        // BIT
	newdecimal = 0xf6, // DECIMAL
	enum_ = 0xf7,      // Apparently not sent
	set = 0xf8,        // Apperently not sent
	tiny_blob = 0xf9,  // Apparently not sent
	medium_blob = 0xfa,// Apparently not sent
	long_blob = 0xfb,  // Apparently not sent
	blob = 0xfc,       // Used for all TEXT and BLOB types
	var_string = 0xfd, // Used for VARCHAR and VARBINARY
	string = 0xfe,     // Used for CHAR and BINARY, ENUM (enum flag set), SET (set flag set)
	geometry = 0xff    // GEOMETRY
};


}



#endif /* INCLUDE_MYSQL_FIELD_TYPE_HPP_ */