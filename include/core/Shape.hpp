#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

class Shape : public Object
{
public:
	/**
	* \brief Uniformly sample a position on the shape with
	* respect to surface area. Returns both position and normal
	*/
	virtual void SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const = 0;

	/// Return the surface area of the given shape
	virtual float SurfaceArea() const = 0;

	/// Return an axis-aligned bounding box of the entire shape
	virtual BoundingBox3f GetBoundingBox() const = 0;

	/// Return the centroid of the given shape
	virtual Point3f GetCentroid() const = 0;

	/** \brief intersection test
	*
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
	virtual bool RayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const = 0;

	/// After intersection test passed, compute the detail information of the intersection point.
	virtual void PostIntersect(Intersection & Isect) const = 0;

	/**
	* \brief Return the pointer of the mesh that this shape attach
	* \return
	*   \c nullptr if the shape does not attach to any mesh
	*/
	virtual Mesh * GetMesh() const;

	/**
	* \brief Return the index of facet in the mesh that this shape attach
	* \return
	*   \c uint32_t(-1) if the shape does not attach to any mesh
	*/
	virtual uint32_t GetFacetIndex() const;

	/// Is this shape an area emitter?
	virtual bool IsEmitter() const;

	/// Return a pointer to an attached area emitter instance
	virtual Emitter * GetEmitter();

	/// Return a pointer to an attached area emitter instance (const version)
	virtual const Emitter * GetEmitter() const;

	/// Return a pointer to the BSDF associated with this shape
	virtual const BSDF * GetBSDF() const = 0;

	/// Compute mean curvature and Gaussian curvature at the specified intersection point
	void ComputeCurvature(const Intersection & Isect, float & H, float & K) const;

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const;
};

NAMESPACE_END