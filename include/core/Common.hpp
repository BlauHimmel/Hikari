#pragma once

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN     /* Don't ever include MFC on Windows */
#define NOMINMAX                /* Don't override min/max */
#endif

#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <cstdint>
#include <stdexcept>

#include <Eigen/Core>
#include <tinyformat.h>
#include <ImathPlatform.h>

#define NAMESPACE_BEGIN namespace Hikari {
#define NAMESPACE_END }

#if defined(MACOS)
	#define PLATFORM_MACOS
#elif defined(LINUX)
	#define PLATFORM_LINUX
#elif defined(WIN32)
	#define PLATFORM_WINDOWS
#endif

/* "Ray epsilon": relative error threshold for ray intersection computations */
#define Epsilon 1e-4

/* A few useful constants */
#undef M_PI

#define M_PI         3.14159265358979323846
#define INV_PI       0.31830988618379067154
#define INV_TWOPI    0.15915494309189533577
#define INV_FOURPI   0.07957747154594766788
#define SQRT_TWO     1.41421356237309504880
#define INV_SQRT_TWO 0.70710678118654752440

/* Import cout, cerr, endl for debugging purposes */
using std::cout;
using std::cerr;
using std::endl;

NAMESPACE_BEGIN

/* Forward declarations */


/* Simple exception class, which stores a human-readable error description */
class HikariException : public std::runtime_error 
{
public:
	// Variadic template constructor to support printf-style arguments
	template <typename... Args> HikariException(const char * pFmt, const Args & ... Args)
		: std::runtime_error(tfm::format(pFmt, Args...)) { }
};

NAMESPACE_END