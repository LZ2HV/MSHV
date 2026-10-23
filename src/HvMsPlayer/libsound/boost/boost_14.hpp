#include "crc.hpp"
#include "config.hpp"
//#define POLY10 0x08f //for 10bit crc
//#define POLY12 0xc06
#define POLY14 0x2757
//#define BOOST_NO_CXX11_CONSTEXPR
#ifdef BOOST_NO_CXX11_CONSTEXPR
//#define TRUNCATED_POLYNOMIAL10 POLY10
//#define TRUNCATED_POLYNOMIAL12 POLY12
#define TRUNCATED_POLYNOMIAL14 POLY14
#else
/*
namespace
{
unsigned long constexpr TRUNCATED_POLYNOMIAL10 = POLY10;
}
namespace
{
unsigned long constexpr TRUNCATED_POLYNOMIAL12 = POLY12;
}
*/
namespace
{
unsigned long constexpr TRUNCATED_POLYNOMIAL14 = POLY14;
}
#endif

// assumes CRC is last 16 bits of the data and is set to zero
// caller should assign the returned CRC into the message in big endian byte order

