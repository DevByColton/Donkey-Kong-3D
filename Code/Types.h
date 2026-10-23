#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cmath>
#include <cstdio>  // NOTE: Really just for debug string printing

//
// Types defs
//
#define internal_link static
#define local_persist static
#define global_variable static

union Vector2
{
	struct
	{
		float x;
		float y;
	};

	float e[2];
};

union Vector4
{
	struct
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct
	{
		float r;
		float g;
		float b;
		float a;
	};

	float e[4];
};

#endif