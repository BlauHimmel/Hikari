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
#include <limits>

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
template <typename TScalar, int TDimension>  struct TVector;
template <typename TScalar, int TDimension>  struct TPoint;
template <typename TPoint, typename TVector> struct TRay;
template <typename TPoint>                   struct TBoundingBox;

typedef TVector<float, 1>       Vector1f;
typedef TVector<float, 2>       Vector2f;
typedef TVector<float, 3>       Vector3f;
typedef TVector<float, 4>       Vector4f;
typedef TVector<double, 1>      Vector1d;
typedef TVector<double, 2>      Vector2d;
typedef TVector<double, 3>      Vector3d;
typedef TVector<double, 4>      Vector4d;
typedef TVector<int, 1>         Vector1i;
typedef TVector<int, 2>         Vector2i;
typedef TVector<int, 3>         Vector3i;
typedef TVector<int, 4>         Vector4i;
typedef TPoint<float, 1>        Point1f;
typedef TPoint<float, 2>        Point2f;
typedef TPoint<float, 3>        Point3f;
typedef TPoint<float, 4>        Point4f;
typedef TPoint<double, 1>       Point1d;
typedef TPoint<double, 2>       Point2d;
typedef TPoint<double, 3>       Point3d;
typedef TPoint<double, 4>       Point4d;
typedef TPoint<int, 1>          Point1i;
typedef TPoint<int, 2>          Point2i;
typedef TPoint<int, 3>          Point3i;
typedef TPoint<int, 4>          Point4i;
typedef TBoundingBox<Point1f>   BoundingBox1f;
typedef TBoundingBox<Point2f>   BoundingBox2f;
typedef TBoundingBox<Point3f>   BoundingBox3f;
typedef TBoundingBox<Point4f>   BoundingBox4f;
typedef TBoundingBox<Point1d>   BoundingBox1d;
typedef TBoundingBox<Point2d>   BoundingBox2d;
typedef TBoundingBox<Point3d>   BoundingBox3d;
typedef TBoundingBox<Point4d>   BoundingBox4d;
typedef TBoundingBox<Point1i>   BoundingBox1i;
typedef TBoundingBox<Point2i>   BoundingBox2i;
typedef TBoundingBox<Point3i>   BoundingBox3i;
typedef TBoundingBox<Point4i>   BoundingBox4i;
typedef TRay<Point2f, Vector2f> Ray2f;
typedef TRay<Point3f, Vector3f> Ray3f;

/* Simple exception class, which stores a human-readable error description */
class HikariException : public std::runtime_error 
{
public:
	// Variadic template constructor to support printf-style arguments
	template <typename... Args> HikariException(const char * pFmt, const Args & ... Args)
		: std::runtime_error(tfm::format(pFmt, Args...)) { }
};

NAMESPACE_END