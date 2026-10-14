#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstdio>  // NOTE: Really just for debug string printing

//
// Types defs
//
#define internal_link static
#define local_persist static
#define global_variable static

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef std::size_t sizet;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

union Vector2
{
	struct
	{
		float x;
		float y;
	};

	float e[2];
};

#endif