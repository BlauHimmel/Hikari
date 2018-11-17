#pragma once

#if defined(_MSC_VER)
/* Disable some warnings on MSVC++ */
#pragma warning(disable : 4127 4702 4100 4515 4800 4146 4512 4819 4334)
#define WIN32_LEAN_AND_MEAN     /* Don't ever include MFC on Windows */
#define NOMINMAX                /* Don't override Min/max */
#endif

#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <thread>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <time.h>

#include <Eigen\Core>
#include <Eigen\Geometry>
#include <Eigen\Lu>
#include <glog\logging.h>
#include <tinyformat.h>
#include <ImathPlatform.h>
#include <filesystem\resolver.h>

#define NAMESPACE_BEGIN namespace Hikari {
#define NAMESPACE_END }

#if defined(MACOS)
	#define __PLATFORM_MACOS__
#elif defined(LINUX)
	#define __PLATFORM_LINUX__
#elif defined(WIN32)
	#define __PLATFORM_WINDOWS__
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

/* XML fields name */
#define XML_TYPE(Field)     Field##_XmlType
#define XML_VALUE(Field)    Field##_XmlValue

#define XML_TRANSFORM_TRANSLATE                  "translate"
#define XML_TRANSFORM_MATRIX                     "matrix"
#define XML_TRANSFORM_ROTATE                     "rotate"
#define XML_TRANSFORM_ANGLE                      "angle"
#define XML_TRANSFORM_AXIS                       "axis"
#define XML_TRANSFORM_SCALE                      "scale"
#define XML_TRANSFORM_LOOKAT                     "lookat"
#define XML_TRANSFORM_ORIGIN                     "origin"
#define XML_TRANSFORM_TARGET                     "target"
#define XML_TRANSFORM_UP                         "up"

#define XML_INTEGRATOR                           "integrator"
#define XML_INTEGRATOR_NORMAL                    "normals"
#define XML_INTEGRATOR_SIMPLE                    "simple"
#define XML_INTEGRATOR_SIMPLE_POSITION           "position"
#define XML_INTEGRATOR_SIMPLE_ENERGY             "energy"
#define XML_INTEGRATOR_AO                        "ao"
#define XML_INTEGRATOR_AO_ALPHA                  "alpha"
#define XML_INTEGRATOR_AO_SAMPLE_COUNT           "sampleCount"
#define XML_INTEGRATOR_WHITTED                   "whitted"

#define XML_EMITTER                              "emitter"
#define XML_EMITTER_AREA_LIGHT                   "area"
#define XML_EMITTER_AREA_LIGHT_RADIANCE          "radiance"

#define XML_ACCELERATION                         "acceleration"
#define XML_ACCELERATION_BRUTO_LOOP              "bruto"
#define XML_ACCELERATION_BVH                     "bvh"
#define XML_ACCELERATION_BVH_LEAF_SIZE           "leafSize"
#define XML_ACCELERATION_BVH_SPLIT_METHOD        "splitMethod"
#define XML_ACCELERATION_BVH_SPLIT_METHOD_CENTER "center"
#define XML_ACCELERATION_BVH_SPLIT_METHOD_SAH    "sah"
#define XML_ACCELERATION_HLBVH                   "hlbvh"
#define XML_ACCELERATION_HLBVH_LEAF_SIZE         "leafSize"

#define XML_SCENE                                "scene"

#define XML_MESH                                 "mesh"
#define XML_MESH_WAVEFRONG_OBJ                   "obj"
#define XML_MESH_WAVEFRONG_OBJ_FILENAME          "filename"
#define XML_MESH_WAVEFRONG_OBJ_TO_WORLD          "toWorld"

#define XML_BSDF                                 "bsdf"
#define XML_BSDF_DIELECTRIC                      "dielectric"
#define XML_BSDF_DIELECTRIC_INT_IOR              "intIOR"
#define XML_BSDF_DIELECTRIC_EXT_IOR              "extIOR"
#define XML_BSDF_DIFFUSE                         "diffuse"
#define XML_BSDF_DIFFUSE_ALBEDO                  "albedo"
#define XML_BSDF_MIRROR                          "mirror"
#define XML_BSDF_MICROFACET                      "microfacet"
#define XML_BSDF_MICROFACET_ALPHA                "alpha"
#define XML_BSDF_MICROFACET_INT_IOR              "intIOR"
#define XML_BSDF_MICROFACET_EXT_IOR              "extIOR"
#define XML_BSDF_MICROFACET_KD                   "kd"

#define XML_MEDIUM                               "medium"

#define XML_PHASE                                "phase"

#define XML_CAMERA                               "camera"
#define XML_CAMERA_PERSPECTIVE                   "perspective"
#define XML_CAMERA_PERSPECTIVE_WIDTH             "width"
#define XML_CAMERA_PERSPECTIVE_HEIGHT            "height"
#define XML_CAMERA_PERSPECTIVE_TO_WORLD          "toWorld"
#define XML_CAMERA_PERSPECTIVE_FOV               "fov"
#define XML_CAMERA_PERSPECTIVE_NEAR_CLIP         "nearClip"
#define XML_CAMERA_PERSPECTIVE_FAR_CLIP          "farClip"

#define XML_TEST                                 "test"

#define XML_FILTER                               "rfilter"
#define XML_FILTER_BOX                           "box"
#define XML_FILTER_GAUSSION                      "gaussian"
#define XML_FILTER_GAUSSION_RADIUS               "radius"
#define XML_FILTER_GAUSSION_STDDEV               "stddev"
#define XML_FILTER_MITCHELL_NETRAVALI            "mitchell"
#define XML_FILTER_MITCHELL_NETRAVALI_RADIUS     "radius"
#define XML_FILTER_MITCHELL_NETRAVALI_B          "B"
#define XML_FILTER_MITCHELL_NETRAVALI_C          "C"
#define XML_FILTER_TENT                          "tent"

#define XML_SAMPLER                              "sampler"
#define XML_SAMPLER_INDEPENDENT                  "independent"
#define XML_SAMPLER_INDEPENDENT_SAMPLE_COUNT     "sampleCount"

#define XML_SHAPE                                "shape"

/* Default setting */
#define DEFAULT_ACCELERATION_BVH_LEAF_SIZE       10
#define DEFAULT_ACCELERATION_BVH_SPLIT_METHOD    XML_ACCELERATION_BVH_SPLIT_METHOD_SAH

#define DEFAULT_ACCELERATION_HLBVH_LEAF_SIZE     10

#define DEFAULT_SCENE_ACCELERATION               XML_ACCELERATION_BRUTO_LOOP

#define DEFAULT_SCENE_SAMPLER                    XML_SAMPLER_INDEPENDENT

#define DEFAULT_CAMERA_OUTPUTSIZE_X              1280
#define DEFAULT_CAMERA_OUTPUTSIZE_Y              720
#define DEFAULT_CAMERA_CAMERA_TO_WORLD           Transform()
#define DEFAULT_CAMERA_FOV                       30.0f
#define DEFAULT_CAMERA_NEAR_CLIP                 1e-4f
#define DEFAULT_CAMERA_FAR_CLIP                  1e4f
#define DEFAULT_CAMERA_FAR_CLIP                  1e4f
#define DEFAULT_CAMERA_RFILTER                   XML_FILTER_GAUSSION

#define DEFAULT_BSDF_DIELECTRIC_INT_IOR          1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_DIELECTRIC_EXT_IOR          1.000277f /* Air */
#define DEFAULT_BSDF_DIFFUSE_ALBEDO              Color3f(0.5f)
#define DEFAULT_BSDF_MICROFACET_ALPHA            0.1f
#define DEFAULT_BSDF_MICROFACET_INT_IOR          1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_MICROFACET_EXT_IOR          1.000277f /* Air */
#define DEFAULT_BSDF_MICROFACET_ALBEDO           Color3f(0.5f)

#define DEFAULT_FILTER_GAUSSIAN_RADIUS           2.0f
#define DEFAULT_FILTER_GAUSSIAN_STDDEV           0.5f
#define DEFAULT_FILTER_MITCHELL_RADIUS           2.0f
#define DEFAULT_FILTER_MITCHELL_B                (1.0f / 3.0f)
#define DEFAULT_FILTER_MITCHELL_C                (1.0f / 3.0f)

#define DEFAULT_MESH_TO_WORLD                     Transform()

#define DEFAULT_INTEGRATOR_AO_ALPHA               1e6f
#define DEFAULT_INTEGRATOR_AO_SAMPLE_COUNT        16

#define DEFAULT_SAMPLER_INDEPENDENT_SAMPLE_COUNT  1

#define DEFAULT_MESH_BSDF                         XML_BSDF_DIFFUSE

NAMESPACE_BEGIN

/* Counting the time between TIMER_START and TIMER_END */
#define TIMER_START(Var) clock_t __Start##Var, __Finish##Var; double Var; __Start##Var = clock();
#define TIMER_END(Var) __Finish##Var = clock();  Var = double(__Finish##Var - __Start##Var) / (CLOCKS_PER_SEC);

/* Import cout, cerr, endl for debugging purposes */
using std::cout;
using std::cerr;
using std::endl;

/* Forward declarations */
template <typename TScalar, int TDimension>  struct TVector;
template <typename TScalar, int TDimension>  struct TPoint;
template <typename TPoint, typename TVector> struct TRay;
template <typename TPoint>                   struct TBoundingBox;

class Acceleration;
class Bitmap;
class ImageBlock;
class BlockGenerator;
class BSDF;
struct BSDFQueryRecord;
class Camera;
struct Color3f;
struct Color4f;
struct DiscretePDF;
class Emitter;
struct Frame;
class Integrator;
class Mesh;
struct Intersection;
class Object;
class ObjectFactory;
class PropertyList;
class ReconstructionFilter;
class Sampler;
class Sampling;
class Scene;
class Timer;
class Shape;

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
	template <typename... Args>
	HikariException(const char * pFmt, const Args &... Arg) : std::runtime_error(tfm::format(pFmt, Arg...)) { }
};

/// Return the number of cores (real and virtual)
int GetCoreCount();

/// Indent a string by the specified number of spaces
std::string Indent(const std::string & String, int Amount = 2);

/// Convert a string to lower case
std::string ToLower(const std::string & String);

/// Convert a string into an boolean value
bool ToBool(const std::string & String);

/// Convert a string into a signed integer value
int ToInt(const std::string & String);

/// Convert a string into an unsigned integer value
unsigned int ToUInt(const std::string & String);

/// Convert a string into a floating point value
float ToFloat(const std::string & String);

/// Convert a string into a 3D vector
Eigen::Vector3f ToVector3f(const std::string & String);

/// Tokenize a string into a list by splitting at 'delim'
std::vector<std::string> Tokenize(const std::string & String, const std::string & Delim = ", ", bool bIncludeEmpty = false);

/// Check if a string ends with another string
bool EndsWith(const std::string & String, const std::string & Ending);

/// Convert a time value in milliseconds into a human-readable string
std::string TimeString(double Time, bool bPrecise = false);

/// Convert a memory amount in bytes into a human-readable string
std::string MemString(size_t Size, bool bPrecise = false);

/// Measures associated with probability distributions
enum class EMeasure
{
	EUnknownMeasure = 0,
	ESolidAngle     = 1,
	EDiscrete       = 2
};

/// Type of the emitter
enum class EEmitterType
{
	EUnknown = 0,
	EPoint = 1,
	EArea = 2,
	EEnvironment = 3
};

/// Convert radians to degrees
inline float RadToDeg(float Value) { return Value * (180.0f / float(M_PI)); }

/// Convert degrees to radians
inline float DegToRad(float Value) { return Value * (float(M_PI) / 180.0f); }

/// Emulate sincosf using sinf() and cosf()
inline void SinCos(float Theta, float * pSin, float * pCos)
{
	*pSin = sinf(Theta);
	*pCos = cosf(Theta);
}

/// Simple floating point clamping function
inline float Clamp(float Value, float Min, float Max)
{
	if (Value < Min)
	{
		return Min;
	}
	else if (Value > Max)
	{
		return Max;
	}
	else
	{
		return Value;
	}
}

/// Simple integer clamping function
inline int Clamp(int Value, int Min, int Max)
{
	if (Value < Min)
	{
		return Min;
	}
	else if (Value > Max)
	{
		return Max;
	}
	else
	{
		return Value;
	}
}

/// Linearly interpolate between two values
inline float Lerp(float T, float V1, float V2)
{
	return (1.0f - T) * V1 + T * V2;
}

/// Always-positive modulo operation
inline int Mod(int A, int B)
{
	int R = A % B;
	return (R < 0) ? R + B : R;
}

/// Compute a direction for the given coordinates in spherical coordinates
Vector3f SphericalDirection(float Theta, float Phi);

/// Compute a direction for the given coordinates in spherical coordinates
Point2f SphericalCoordinates(const Vector3f & Dir);

/**
* \brief Calculates the unpolarized fresnel reflection coefficient for a
* dielectric material. Handles incidence from either side (i.e.
* \code CosThetaI < 0 is allowed).
*
* \param CosThetaI
*      Cosine of the angle between the normal and the incident ray
* \param ExtIOR
*      Refractive index of the side that contains the surface normal
* \param IntIOR
*      Refractive index of the interior
*/
float Fresnel(float CosThetaI, float ExtIOR, float IntIOR);

/// Complete the set {a} to an orthonormal base
void CoordinateSystem(const Vector3f & Va, Vector3f & Vb, Vector3f & Vc);

/**
* \brief Return the global file resolver instance
*
* This class is used to locate resource files (e.g. mesh or
* texture files) referenced by a scene being loaded
*/
filesystem::resolver * GetFileResolver();

NAMESPACE_END