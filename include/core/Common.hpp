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

NAMESPACE_BEGIN

/* Import cout, cerr, endl for debugging purposes */
using std::cout;
using std::cerr;
using std::endl;

/* Forward declarations */
template <typename TScalar, int TDimension>  struct TVector;
template <typename TScalar, int TDimension>  struct TPoint;
template <typename TPoint, typename TVector> struct TRay;
template <typename TPoint>                   struct TBoundingBox;

/* Basic data structures (vectors, points, rays, bounding boxes,
kd-trees) are oblivious to the underlying data type and dimension.
The following list of typedefs establishes some convenient aliases
for specific types. */
using Vector1f      = TVector<float, 1>;
using Vector2f      = TVector<float, 2>;
using Vector3f      = TVector<float, 3>;
using Vector4f      = TVector<float, 4>;
using Vector1d      = TVector<double, 1>;
using Vector2d      = TVector<double, 2>;
using Vector3d      = TVector<double, 3>;
using Vector4d      = TVector<double, 4>;
using Vector1i      = TVector<int, 1>;
using Vector2i      = TVector<int, 2>;
using Vector3i      = TVector<int, 3>;
using Vector4i      = TVector<int, 4>;
using Point1f       = TPoint<float, 1>;
using Point2f       = TPoint<float, 2>;
using Point3f       = TPoint<float, 3>;
using Point4f       = TPoint<float, 4>;
using Point1d       = TPoint<double, 1>;
using Point2d       = TPoint<double, 2>;
using Point3d       = TPoint<double, 3>;
using Point4d       = TPoint<double, 4>;
using Point1i       = TPoint<int, 1>;
using Point2i       = TPoint<int, 2>;
using Point3i       = TPoint<int, 3>;
using Point4i       = TPoint<int, 4>;
using BoundingBox1f = TBoundingBox<Point1f>;
using BoundingBox2f = TBoundingBox<Point2f>;
using BoundingBox3f = TBoundingBox<Point3f>;
using BoundingBox4f = TBoundingBox<Point4f>;
using BoundingBox1d = TBoundingBox<Point1d>;
using BoundingBox2d = TBoundingBox<Point2d>;
using BoundingBox3d = TBoundingBox<Point3d>;
using BoundingBox4d = TBoundingBox<Point4d>;
using BoundingBox1i = TBoundingBox<Point1i>;
using BoundingBox2i = TBoundingBox<Point2i>;
using BoundingBox3i = TBoundingBox<Point3i>;
using BoundingBox4i = TBoundingBox<Point4i>;
using Ray2f         = TRay<Point2f, Vector2f>;
using Ray3f         = TRay<Point3f, Vector3f>;
using MatrixXf      = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXu      = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;

/* Simple exception class, which stores a human-readable error description */
class HikariException : public std::runtime_error 
{
public:
	// Variadic template constructor to support printf-style arguments
	template <typename... Args> HikariException(const char * pFmt, const Args & ... Args)
		: std::runtime_error(tfm::format(pFmt, Args...)) { }
};

NAMESPACE_END