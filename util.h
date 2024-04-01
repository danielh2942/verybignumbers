/*
 * File:      util.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon 
 */

#include <ostream>
#ifndef UTIL_H_E5EC9B1BB57E4492AFB17BCAF1223F53
#define UTIL_H_E5EC9B1BB57E4492AFB17BCAF1223F53 1

#include <string>
#include <cstdint>
#include <type_traits>

std::string get_uuid();

// This is a concept to check if something is a sign agnostic
// width 32 integer
template<class T>
concept width32int = (std::is_same_v<T,std::uint32_t> || std::is_same_v<T,std::int32_t>);

// Check if hex mode
inline bool stream_hex_mode(std::ostream & os) {
	return (os.flags() & std::ostream::hex) != 0;
}

#endif // UTIL_H_E5EC9B1BB57E4492AFB17BCAF1223F53
	
