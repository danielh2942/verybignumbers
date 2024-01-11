#include <cstdint>

inline int const clz_uint8(std::uint8_t const& val) {
	//128
	if(val >= 128) return 0;
	//64
	if(val >= 64) return 1;
	//32
	if(val >= 32) return 2;
	//16
	if(val >= 16) return 3;
	//8
	if(val >= 8) return 4;
	//4
	if(val >= 4) return 5;
	//2
	if(val >= 2) return 6;
	//1
	if(val >= 1) return 7;
	return 8;
}
