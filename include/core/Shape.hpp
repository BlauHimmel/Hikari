#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\Vector.hpp>

NAMESPACE_BEGIN

class Shape : public Object
{
public:
	/**
	* \brief Uniformly sample a position on the mesh with
	* respect to surface area. Returns both position and normal
	*/
	virtual void SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const = 0;

	/// Return the surface area of the given triangle
	virtual float SurfaceArea() const = 0;

	/// Return an axis-aligned bounding box of the entire mesh
	virtual const BoundingBox3f & GetBoundingBox() const = 0;

	/// Return the centroid of the given triangle
	virtual Point3f GetCentroid() const = 0;

	/** \brief intersection test
	*
	* \param Index
	*    Index of the triangle that should be intersected
	* \param Ray
	*    The ray segment to be used for the intersection query
	* \param U
	*   Upon success, \c U will contain the 'U' component of the intersection
	*   in barycentric coordinates
	* \param V
	*   Upon success, \c V will contain the 'V' component of the intersection
	*   in barycentric coordinates
	* \param T
	*    Upon success, \a T contains the distance from the ray origin to the
	*    intersection point,
	* \return
	*   \c true if an intersection has been detected
	*/
	virtual bool RayIntersect(uint32_t Index, const Ray3f & Ray, float & U, float & V, float & T) const = 0;

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const;
};

NAMESPACE_END