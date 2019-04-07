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

/* Used for BSDF::Eval() or BSDF::PDF() for the delta distribution. */
#define DeltaEpsilon 1e-3

/* To avoid the numerical error in computation */
#define MIN_ALPHA 5e-4
#define MAX_ALPHA 1.0

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
#define XML_INTEGRATOR_SIMPLE_POWER              "power"
#define XML_INTEGRATOR_AO                        "ao"
#define XML_INTEGRATOR_AO_ALPHA                  "alpha"
#define XML_INTEGRATOR_AO_SAMPLE_COUNT           "sampleCount"
#define XML_INTEGRATOR_WHITTED                   "whitted"
#define XML_INTEGRATOR_WHITTED_DEPTH             "depth"
#define XML_INTEGRATOR_PATH_EMS                  "pathEMS"
#define XML_INTEGRATOR_PATH_EMS_DEPTH            "depth"
#define XML_INTEGRATOR_PATH_MATS                 "pathMATS"
#define XML_INTEGRATOR_PATH_MATS_DEPTH           "depth"
#define XML_INTEGRATOR_PATH_MIS                  "pathMIS"
#define XML_INTEGRATOR_PATH_MIS_DEPTH            "depth"

#define XML_EMITTER                              "emitter"
#define XML_EMITTER_AREA_LIGHT                   "area"
#define XML_EMITTER_AREA_LIGHT_RADIANCE          "radiance"
#define XML_EMITTER_POINT_LIGHT                  "point"
#define XML_EMITTER_POINT_LIGHT_POSITION         "position"
#define XML_EMITTER_POINT_LIGHT_POWER            "power"
#define XML_EMITTER_ENVIRONMENT_LIGHT            "env"
#define XML_EMITTER_ENVIRONMENT_LIGHT_FILENAME   "filename"
#define XML_EMITTER_ENVIRONMENT_LIGHT_SCALE      "scale"
#define XML_EMITTER_ENVIRONMENT_LIGHT_TO_WORLD   "toWorld"
#define XML_EMITTER_DIRECTIONAL_LIGHT            "directional"
#define XML_EMITTER_DIRECTIONAL_LIGHT_POWER      "power"
#define XML_EMITTER_DIRECTIONAL_LIGHT_DIRECTION  "direction"
#define XML_EMITTER_CONSTANT_LIGHT               "constant"
#define XML_EMITTER_CONSTANT_LIGHT_RADIANCE      "radiance"

#define XML_TEXTURE                              "texture"
#define XML_TEXTURE_BITMAP                       "bitmap"
#define XML_TEXTURE_BITMAP_FILENAME              "filename"
#define XML_TEXTURE_BITMAP_GAMMA                 "gamma"
#define XML_TEXTURE_BITMAP_WRAP_MODE             "wrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_U           "uWrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_V           "vWrapMode"
#define XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT      "repeat"
#define XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP       "clamp"
#define XML_TEXTURE_BITMAP_WRAP_MODE_BLACK       "black"
#define XML_TEXTURE_BITMAP_FILTER_TYPE           "filterType"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_NEAREST   "nearest"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_BILINEAR  "bilinear"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_TRILINEAR "trilinear"
#define XML_TEXTURE_BITMAP_FILTER_TYPE_EWA       "ewa"
#define XML_TEXTURE_BITMAP_MAX_ANISOTROPY        "maxAnisotropy"
#define XML_TEXTURE_BITMAP_OFFSE_U               "uOffset"
#define XML_TEXTURE_BITMAP_OFFSE_V               "vOffset"
#define XML_TEXTURE_BITMAP_SCALE_U               "uScale"
#define XML_TEXTURE_BITMAP_SCALE_V               "vScale"
#define XML_TEXTURE_BITMAP_CHANNEL               "channel"
#define XML_TEXTURE_BITMAP_CHANNEL_R             "r"
#define XML_TEXTURE_BITMAP_CHANNEL_RGB           "rgb"
#define XML_TEXTURE_CHECKERBOARD                 "checkerboard"
#define XML_TEXTURE_CHECKERBOARD_BLOCKS          "blocks"
#define XML_TEXTURE_CHECKERBOARD_COLOR_A         "colorA"
#define XML_TEXTURE_CHECKERBOARD_COLOR_B         "colorB"
#define XML_TEXTURE_CHECKERBOARD_OFFSE_U         "uOffset"
#define XML_TEXTURE_CHECKERBOARD_OFFSE_V         "vOffset"
#define XML_TEXTURE_CHECKERBOARD_SCALE_U         "uScale"
#define XML_TEXTURE_CHECKERBOARD_SCALE_V         "vScale"
#define XML_TEXTURE_WIREFRAME                    "wireframe"
#define XML_TEXTURE_WIREFRAME_INTERIOR_COLOR     "interiorColor"
#define XML_TEXTURE_WIREFRAME_EDGE_COLOR         "edgeColor"
#define XML_TEXTURE_WIREFRAME_EDGE_WIDTH         "edgeWidth"
#define XML_TEXTURE_WIREFRAME_TRANSITION_WIDTH   "transitionWidth"
#define XML_TEXTURE_WIREFRAME_OFFSE_U            "uOffset"
#define XML_TEXTURE_WIREFRAME_OFFSE_V            "vOffset"
#define XML_TEXTURE_WIREFRAME_SCALE_U            "uScale"
#define XML_TEXTURE_WIREFRAME_SCALE_V            "vScale"
#define XML_TEXTURE_GRID                         "grid"
#define XML_TEXTURE_GRID_COLOR_BACKGROUND        "backgroundColor"
#define XML_TEXTURE_GRID_COLOR_LINE              "lineColor"
#define XML_TEXTURE_GRID_LINE_WIDTH              "lineWidth"
#define XML_TEXTURE_GRID_LINES                   "lines"
#define XML_TEXTURE_GRID_OFFSE_U                 "uOffset"
#define XML_TEXTURE_GRID_OFFSE_V                 "vOffset"
#define XML_TEXTURE_GRID_SCALE_U                 "uScale"
#define XML_TEXTURE_GRID_SCALE_V                 "vScale"

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
#define XML_SCENE_BACKGROUND                     "background"
#define XML_SCENE_FORCE_BACKGROUND               "forceBackground"

#define XML_MESH                                 "mesh"
#define XML_MESH_WAVEFRONG_OBJ                   "obj"
#define XML_MESH_WAVEFRONG_OBJ_FILENAME          "filename"
#define XML_MESH_WAVEFRONG_OBJ_TO_WORLD          "toWorld"

#define XML_BSDF                                 "bsdf"
#define XML_BSDF_GGX                             "ggx"
#define XML_BSDF_BECKMANN                        "beckmann"
#define XML_BSDF_DIELECTRIC                      "dielectric"
#define XML_BSDF_DIELECTRIC_INT_IOR              "intIOR"
#define XML_BSDF_DIELECTRIC_EXT_IOR              "extIOR"
#define XML_BSDF_DIELECTRIC_KS_REFLECT           "ksReflect"
#define XML_BSDF_DIELECTRIC_KS_REFRACT           "ksRefract"
#define XML_BSDF_DIFFUSE                         "diffuse"
#define XML_BSDF_DIFFUSE_ALBEDO                  "albedo"
#define XML_BSDF_MIRROR                          "mirror"
#define XML_BSDF_MICROFACET                      "microfacet"
#define XML_BSDF_MICROFACET_ALPHA                "alpha"
#define XML_BSDF_MICROFACET_INT_IOR              "intIOR"
#define XML_BSDF_MICROFACET_EXT_IOR              "extIOR"
#define XML_BSDF_MICROFACET_KD                   "kd"
#define XML_BSDF_CONDUCTOR                       "conductor"
#define XML_BSDF_CONDUCTOR_INT_IOR               "intIOR"
#define XML_BSDF_CONDUCTOR_EXT_IOR               "extIOR"
#define XML_BSDF_CONDUCTOR_K                     "k"
#define XML_BSDF_CONDUCTOR_KS                    "ks"
#define XML_BSDF_PLASTIC                         "plastic"
#define XML_BSDF_PLASTIC_INT_IOR                 "intIOR"
#define XML_BSDF_PLASTIC_EXT_IOR                 "extIOR"
#define XML_BSDF_PLASTIC_KS                      "ks"
#define XML_BSDF_PLASTIC_KD                      "kd"
#define XML_BSDF_PLASTIC_NONLINEAR               "nonlinear"
#define XML_BSDF_ROUGH_CONDUCTOR                 "roughConductor"
#define XML_BSDF_ROUGH_CONDUCTOR_INT_IOR         "intIOR"
#define XML_BSDF_ROUGH_CONDUCTOR_EXT_IOR         "extIOR"
#define XML_BSDF_ROUGH_CONDUCTOR_K               "k"
#define XML_BSDF_ROUGH_CONDUCTOR_KS              "ks"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA           "alpha"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U         "alphaU"
#define XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V         "alphaV"
#define XML_BSDF_ROUGH_CONDUCTOR_TYPE            "type"
#define XML_BSDF_ROUGH_CONDUCTOR_AS              "as"
#define XML_BSDF_ROUGH_DIELECTRIC                "roughDielectric"
#define XML_BSDF_ROUGH_DIELECTRIC_INT_IOR        "intIOR"
#define XML_BSDF_ROUGH_DIELECTRIC_EXT_IOR        "extIOR"
#define XML_BSDF_ROUGH_DIELECTRIC_KS_REFLECT     "ksReflect"
#define XML_BSDF_ROUGH_DIELECTRIC_KS_REFRACT     "ksRefract"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA          "alpha"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA_U        "alphaU"
#define XML_BSDF_ROUGH_DIELECTRIC_ALPHA_V        "alphaV"
#define XML_BSDF_ROUGH_DIELECTRIC_TYPE           "type"
#define XML_BSDF_ROUGH_DIELECTRIC_AS             "as"
#define XML_BSDF_ROUGH_DIFFUSE                   "roughDiffuse"
#define XML_BSDF_ROUGH_DIFFUSE_ALBEDO            "albedo"
#define XML_BSDF_ROUGH_DIFFUSE_ALPHA             "alpha"
#define XML_BSDF_ROUGH_DIFFUSE_FAST_APPROX       "fastApprox"

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
#define XML_TEST_STUDENT_T                       "ttest"
#define XML_TEST_STUDENT_T_SIGNIFICANCE_LEVEL    "significanceLevel"
#define XML_TEST_STUDENT_T_ANGLES                "angles"
#define XML_TEST_STUDENT_T_REFERENCES            "references"
#define XML_TEST_STUDENT_T_SAMPLE_COUNT          "sampleCount"
#define XML_TEST_CHI2                            "chi2test"
#define XML_TEST_CHI2_SIGNIFICANCE_LEVEL         "significanceLevel"
#define XML_TEST_CHI2_RESOLUTION                 "resolution"
#define XML_TEST_CHI2_MIN_EXP_FREQUENCY          "minExpFrequency"
#define XML_TEST_CHI2_SAMPLE_COUNT               "sampleCount"
#define XML_TEST_CHI2_TEST_COUNT                 "testCount"

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
#define DEFAULT_ACCELERATION_BVH_LEAF_SIZE         10
#define DEFAULT_ACCELERATION_BVH_SPLIT_METHOD      XML_ACCELERATION_BVH_SPLIT_METHOD_SAH

#define DEFAULT_ACCELERATION_HLBVH_LEAF_SIZE       10

#define DEFAULT_SCENE_ACCELERATION                 XML_ACCELERATION_BRUTO_LOOP

#define DEFAULT_SCENE_SAMPLER                      XML_SAMPLER_INDEPENDENT

#define DEFAULT_CAMERA_OUTPUTSIZE_X                1280
#define DEFAULT_CAMERA_OUTPUTSIZE_Y                720
#define DEFAULT_CAMERA_CAMERA_TO_WORLD             Transform()
#define DEFAULT_CAMERA_FOV                         30.0f
#define DEFAULT_CAMERA_NEAR_CLIP                   1e-4f
#define DEFAULT_CAMERA_FAR_CLIP                    1e4f
#define DEFAULT_CAMERA_FAR_CLIP                    1e4f
#define DEFAULT_CAMERA_RFILTER                     XML_FILTER_GAUSSION

#define DEFAULT_BSDF_DIELECTRIC_INT_IOR            1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_DIELECTRIC_EXT_IOR            1.000277f /* Air */
#define DEFAULT_BSDF_DIELECTRIC_KS_REFLECT         Color3f(1.0f) 
#define DEFAULT_BSDF_DIELECTRIC_KS_REFRACT         Color3f(1.0f) 
#define DEFAULT_BSDF_DIFFUSE_ALBEDO                Color3f(0.5f)
#define DEFAULT_BSDF_MICROFACET_ALPHA              0.1f
#define DEFAULT_BSDF_MICROFACET_INT_IOR            1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_MICROFACET_EXT_IOR            1.000277f /* Air */
#define DEFAULT_BSDF_MICROFACET_ALBEDO             Color3f(0.5f)
#define DEFAULT_BSDF_CONDUCTOR_INT_IOR             1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_CONDUCTOR_EXT_IOR             1.000277f /* Air */
#define DEFAULT_BSDF_CONDUCTOR_K                   Color3f(1.0f)
#define DEFAULT_BSDF_CONDUCTOR_KS                  Color3f(1.0f)
#define DEFAULT_BSDF_PLASTIC_INT_IOR               1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_PLASTIC_EXT_IOR               1.000277f /* Air */
#define DEFAULT_BSDF_PLASTIC_KS                    Color3f(1.0f)
#define DEFAULT_BSDF_PLASTIC_KD                    Color3f(0.5f)
#define DEFAULT_BSDF_PLASTIC_NONLINEAR             false
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_INT_IOR       1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_EXT_IOR       1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_K             Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_KS            Color3f(1.0f)
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA         0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_U       0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_V       0.1f
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_TYPE          XML_BSDF_BECKMANN
#define DEFAULT_BSDF_ROUGH_CONDUCTOR_AS            false
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_INT_IOR      1.5046f /* (default: BK7 borosilicate optical glass) */
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_EXT_IOR      1.000277f /* Air */
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFLECT   Color3f(1.0f) 
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_KS_REFRACT   Color3f(1.0f) 
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA        0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_U      0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_ALPHA_V      0.1f
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_TYPE         XML_BSDF_BECKMANN
#define DEFAULT_BSDF_ROUGH_DIELECTRIC_AS           false
#define DEFAULT_BSDF_ROUGH_DIFFUSE_ALBEDO          Color3f(0.5f)
#define DEFAULT_BSDF_ROUGH_DIFFUSE_ALPHA           0.2f
#define DEFAULT_BSDF_ROUGH_DIFFUSE_FAST_APPROX     false

#define DEFAULT_FILTER_GAUSSIAN_RADIUS             2.0f
#define DEFAULT_FILTER_GAUSSIAN_STDDEV             0.5f
#define DEFAULT_FILTER_MITCHELL_RADIUS             2.0f
#define DEFAULT_FILTER_MITCHELL_B                  (1.0f / 3.0f)
#define DEFAULT_FILTER_MITCHELL_C                  (1.0f / 3.0f)

#define DEFAULT_MESH_TO_WORLD                      Transform()

#define DEFAULT_INTEGRATOR_AO_ALPHA                1e6f
#define DEFAULT_INTEGRATOR_AO_SAMPLE_COUNT         16
#define DEFAULT_INTEGRATOR_WHITTED_DEPTH           -1

#define DEFAULT_SAMPLER_INDEPENDENT_SAMPLE_COUNT   1

#define DEFAULT_MESH_BSDF                          XML_BSDF_DIFFUSE

#define DEFAULT_EMITTER_ENVIRONMENT_SCALE          1.0f
#define DEFAULT_EMITTER_ENVIRONMENT_TO_WORLD       Transform()

#define DEFAULT_TEST_STUDENT_T_SIGNIFICANCE_LEVEL  0.01f
#define DEFAULT_TEST_STUDENT_T_ANGLES              ""
#define DEFAULT_TEST_STUDENT_T_REFERENCES          ""
#define DEFAULT_TEST_STUDENT_T_SAMPLE_COUNT        100000

#define DEFAULT_TEST_CHI2_SIGNIFICANCE_LEVEL       0.01f
#define DEFAULT_TEST_CHI2_RESOLUTION               10
#define DEFAULT_TEST_CHI2_MIN_EXP_FREQUENCY        5
#define DEFAULT_TEST_CHI2_SAMPLE_COUNT             -1
#define DEFAULT_TEST_CHI2_TEST_COUNT               5

#define DEFAULT_SCENE_BACKGROUND                   Color3f(0.0f)
#define DEFAULT_SCENE_FORCE_BACKGROUND             false

#define DEFAULT_TEXTURE_BITMAP_GAMMA               1.0f
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE           XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE_U         XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_WRAP_MODE_V         XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT
#define DEFAULT_TEXTURE_BITMAP_FILTER_TYPE         XML_TEXTURE_BITMAP_FILTER_TYPE_EWA
#define DEFAULT_TEXTURE_BITMAP_MAX_ANISOTROPY      20.0f
#define DEFAULT_TEXTURE_BITMAP_OFFSET_U            0.0f
#define DEFAULT_TEXTURE_BITMAP_OFFSET_V            0.0f
#define DEFAULT_TEXTURE_BITMAP_SCALE_U             1.0f
#define DEFAULT_TEXTURE_BITMAP_SCALE_V             1.0f
#define DEFAULT_TEXTURE_BITMAP_CHANNEL             XML_TEXTURE_BITMAP_CHANNEL_RGB
#define DEFAULT_TEXTURE_CHECKERBOARD_BLOCKS        10
#define DEFAULT_TEXTURE_CHECKERBOARD_COLOR_A       Color3f(0.4f)
#define DEFAULT_TEXTURE_CHECKERBOARD_COLOR_B       Color3f(0.2f)
#define DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_U      0.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_V      0.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_SCALE_U       1.0f
#define DEFAULT_TEXTURE_CHECKERBOARD_SCALE_V       1.0f
#define DEFAULT_TEXTURE_WIREFRAME_INTERIOR_COLOR   Color3f(0.5f)
#define DEFAULT_TEXTURE_WIREFRAME_EDGE_COLOR       Color3f(0.1f)
#define DEFAULT_TEXTURE_WIREFRAME_EDGE_WIDTH       0.0f
#define DEFAULT_TEXTURE_WIREFRAME_TRANSITION_WIDTH 0.5f
#define DEFAULT_TEXTURE_WIREFRAME_OFFSET_U         0.0f
#define DEFAULT_TEXTURE_WIREFRAME_OFFSET_V         0.0f
#define DEFAULT_TEXTURE_WIREFRAME_SCALE_U          1.0f
#define DEFAULT_TEXTURE_WIREFRAME_SCALE_V          1.0f
#define DEFAULT_TEXTURE_GRID_COLOR_BACKGROUND      Color3f(0.2f)
#define DEFAULT_TEXTURE_GRID_COLOR_LINE            Color3f(0.4f)
#define DEFAULT_TEXTURE_GRID_LINE_WIDTH            0.01f
#define DEFAULT_TEXTURE_GRID_LINES                 10
#define DEFAULT_TEXTURE_GRID_OFFSET_U              0.0f
#define DEFAULT_TEXTURE_GRID_OFFSET_V              0.0f
#define DEFAULT_TEXTURE_GRID_SCALE_U               1.0f
#define DEFAULT_TEXTURE_GRID_SCALE_V               1.0f

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
template <typename T>                        class MipMap;

class Acceleration;
class Bitmap;
class ImageBlock;
class BlockGenerator;
class BSDF;
struct BSDFQueryRecord;
class Camera;
struct Color3f;
struct Color4f;
struct DiscretePDF1D;
struct DiscretePDF2D;
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
class MicrofacetDistribution;
class Texture;
class Texture2D;
class ConstantColor3fTexture;
class ConstantFloatTexture;
class Color3fAdditionTexture;
class Color3fSubtractionTexture;
class Color3fProductTexture;

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
using MipMap3f      = MipMap<Color3f>;
using MipMap1f      = MipMap<float>;

/* Simple exception class, which stores a human-readable error description */
class HikariException : public std::runtime_error 
{
public:
	// Variadic template constructor to support printf-style arguments
	template <typename... Args>
	HikariException(const char * pFmt, const Args &... Arg) : std::runtime_error(tfm::format(pFmt, Arg...)) { }
};

template<class T>
void CheckRange(const std::string & ObjName, const std::string & FieldName, T Var, T Min, T Max)
{
	if (Var >= Min && Var <= Max)
	{
		return;
	}
	throw HikariException(tfm::format("Field %s in object %s out of range. (%s, %s)"), FieldName, ObjName, Min.ToString(), Max.ToString());
}

template<>
inline void CheckRange<int>(const std::string & ObjName, const std::string & FieldName, int Var, int Min, int Max)
{
	if (Var >= Min && Var <= Max)
	{
		return;
	}
	throw HikariException("Field %s in object %s out of range (%d, %d).", FieldName, ObjName, Min, Max);
}

template<>
inline void CheckRange<uint32_t>(const std::string & ObjName, const std::string & FieldName, uint32_t Var, uint32_t Min, uint32_t Max)
{
	if (Var >= Min && Var <= Max)
	{
		return;
	}
	throw HikariException("Field %s in object %s out of range (%u, %u).", FieldName, ObjName, Min, Max);
}

template<>
inline void CheckRange<float>(const std::string & ObjName, const std::string & FieldName, float Var, float Min, float Max)
{
	if (Var >= Min && Var <= Max)
	{
		return;
	}
	throw HikariException("Field %s in object %s out of range (%f, %f).", FieldName, ObjName, Min, Max);
}

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
	EEnvironment = 3,
	EDirectional = 4
};

/// Type of the light transport. Currently not used, so set it to ERadiance now. 
enum class ETransportMode
{
	EUnknown = 0,
	ERadiance = 1,
	EImportance = 2
};

/// Wrap mode of the texture, which decides the behavior when u > 1 or v > 1.
enum class EWrapMode
{
	ERepeat, EBlack, EClamp
};

/// Filter type of the sampler
enum class EFilterType
{
	ENearest, EBilinear, ETrilinear, EEWA
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

Color3f Clamp(Color3f Value, Color3f Min, Color3f Max);
Color4f Clamp(Color4f Value, Color4f Min, Color4f Max);

/// Linearly interpolate between two values
inline float Lerp(float T, float V1, float V2)
{
	return (1.0f - T) * V1 + T * V2;
}

/// Linearly interpolate between two Color
Color3f Lerp(float T, const Color3f & V1, const Color3f & V2);
Color4f Lerp(float T, const Color4f & V1, const Color4f & V2);

/// Always-positive modulo operation
inline int Mod(int A, int B)
{
	int R = A % B;
	return (R < 0) ? R + B : R;
}

/// Arcsine variant that gracefully handles arguments > 1 that are due to roundoff errors
inline float SafeAsin(float Value)
{
	return std::asin(std::min(1.0f, std::max(-1.0f, Value)));
}

/// Arcsine variant that gracefully handles arguments > 1 that are due to roundoff errors
inline double SafeAsin(double Value)
{
	return std::asin(std::min(1.0, std::max(-1.0, Value)));
}

/// Arccosine variant that gracefully handles arguments > 1 that are due to roundoff errors
inline float SafeAcos(float Value)
{
	return std::acos(std::min(1.0f, std::max(-1.0f, Value)));
}

/// Arccosine variant that gracefully handles arguments > 1 that are due to roundoff errors
inline double SafeAcos(double Value)
{
	return std::acos(std::min(1.0, std::max(-1.0, Value)));
}

/// Square root variant that gracefully handles arguments < 0 that are due to roundoff errors
inline float SafeSqrt(float Value)
{
	return std::sqrt(std::max(0.0f, Value));
}

/// Square root variant that gracefully handles arguments < 0 that are due to roundoff errors
inline double SafeSqrt(double Value)
{
	return std::sqrt(std::max(0.0, Value));
}

inline float Hypot2(float A, float B)
{
	float Result;
	if (std::abs(A) > std::abs(B))
	{
		Result = B / A;
		Result = std::abs(A) * std::sqrt(1.0f + Result * Result);
	}
	else if (B != 0.0f)
	{
		Result = A / B;
		Result = std::abs(B) * std::sqrt(1.0f + Result * Result);
	}
	else
	{
		Result = 0.0f;
	}
	return Result;
}

inline double Hypot2(double A, double B)
{
	double Result;
	if (std::abs(A) > std::abs(B))
	{
		Result = B / A;
		Result = std::abs(A) * std::sqrt(1.0 + Result * Result);
	}
	else if (B != 0.0)
	{
		Result = A / B;
		Result = std::abs(B) * std::sqrt(1.0 + Result * Result);
	}
	else
	{
		Result = 0.0;
	}
	return Result;
}

inline bool SolveLinearSystem2x2(const float A[2][2], const float B[2], float X[2])
{
	float Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

	constexpr float InvOverflow = 1.0f / std::numeric_limits<float>::max();

	if (std::abs(Det) <= InvOverflow)
	{
		return false;
	}

	float InvDet = 1.0f / Det;

	X[0] = (A[1][1] * B[0] - A[0][1] * B[1]) * InvDet;
	X[1] = (A[0][0] * B[1] - A[1][0] * B[0]) * InvDet;

	return true;
}

/// Simple signum function -- note that it returns the FP sign of the input (and never zero)
inline float Signum(float Value)
{
#if defined(__PLATFORM_WINDOWS__)
	return float(_copysign(1.0f, Value));
#elif
	return float(copysign(1.0f, Value));
#endif
}

template <typename T>
inline constexpr bool IsPowerOf2(T Value)
{
	return Value && !(Value & (Value - 1));
}

inline int32_t RoundUpPow2(int32_t Value)
{
	Value--;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	return Value + 1;
}

inline int64_t RoundUpPow2(int64_t Value)
{
	Value--;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	Value |= Value >> 32;
	return Value + 1;
}

inline int Log2Int(uint32_t V)
{
#if defined(__PLATFORM_WINDOWS__)
	unsigned long LZ = 0;
	if (_BitScanReverse(&LZ, V)) { return LZ; }
	return 0;
#else
	return 31 - __builtin_clz(V);
#endif
}

inline int Log2Int(int32_t V)
{
	return Log2Int(uint32_t(V));
}

inline int Log2Int(uint64_t V)
{
#if defined(__PLATFORM_WINDOWS__)
	unsigned long LZ = 0;
#if defined(_WIN64)
	_BitScanReverse64(&LZ, V);
#else
	if (_BitScanReverse(&LZ, V >> 32))
	{
		LZ += 32;
	}
	else
	{
		_BitScanReverse(&LZ, V & 0xffffffff);
	}
#endif
	return LZ;
#else 
	return 63 - __builtin_clzll(V);
#endif
}

inline int Log2Int(int64_t V)
{
	return Log2Int(uint64_t(V));
}

inline float GammaCorrect(float Value, float InvGamma)
{
	return InvGamma == 1.0f ? Value : std::pow(Value, InvGamma);
}

inline int ModPositive(int A, int B)
{
	int R = A % B;
	return (R < 0) ? R + B : R;
}

/// S-shape interpolation between two values
inline float SmoothStep(float Min, float Max, float Value)
{
	float V = Clamp((Value - Min) / (Max - Min), 0.0f, 1.0f);
	return V * V * (-2.0f * V + 3.0f);
}

/// Compute a direction for the given coordinates in spherical coordinates
Vector3f SphericalDirection(float Theta, float Phi);

/// Compute a direction for the given coordinates in spherical coordinates
Point2f SphericalCoordinates(const Vector3f & Dir);

/// Fresnel coefficient for dielectric material. Eta = IntIOR / ExtIOR
float FresnelDielectric(float CosThetaI, float Eta, float InvEta, float & CosThetaT);

/// Fresnel coefficient for conductor material. Eta = IntIOR / ExtIOR, EtaK = K / ExtIOR
Color3f FresnelConductor(float CosThetaI, const Color3f & Eta, const Color3f & EtaK);

/**
* Approximating the diffuse Frensel reflectance 
* for the Eta < 1.0 and Eta > 1.0 cases.
*/
float ApproxFresnelDiffuseReflectance(float Eta);

/// Complete the set {a} to an orthonormal base
void CoordinateSystem(const Vector3f & Va, Vector3f & Vb, Vector3f & Vc);

/// Reflection in local coordinates
Vector3f Reflect(const Vector3f & Wi);

/// Refraction in local coordinates
Vector3f Refract(const Vector3f & Wi, float CosThetaT, float Eta, float InvEta);

/// Reflection in global coordinates
Vector3f Reflect(const Vector3f & Wi, const Vector3f & M);

/// Refraction in local coordinates
Vector3f Refract(const Vector3f & Wi, const Vector3f & M, float CosThetaT, float Eta, float InvEta);

/**
* \brief Return the global file resolver instance
*
* This class is used to locate resource files (e.g. mesh or
* texture files) referenced by a scene being loaded
*/
filesystem::resolver * GetFileResolver();

NAMESPACE_END