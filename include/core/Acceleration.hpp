#pragma once

#include <core\Common.hpp>
#include <core\Mesh.hpp>

NAMESPACE_BEGIN

/**
* \brief Acceleration data structure for ray intersection queries
*
* The current implementation falls back to a brute force loop
* through the geometry.
*/
class Acceleration
{
public:
	/**
	* \brief Register a triangle mesh for inclusion in the acceleration
	* data structure
	*
	* This function can only be used before \ref Build() is called
	*/
	void AddMesh(Mesh * pMesh);

	/// Build the acceleration data structure (currently a no-op)
	void Build();

	/// Return an axis-aligned box that bounds the scene
	const BoundingBox3f & GetBoundingBox() const;

	/**
	* \brief Intersect a ray against all triangles stored in the scene and
	* return detailed intersection information
	*
	* \param Ray
	*    A 3-dimensional ray data structure with minimum/maximum extent
	*    information
	*
	* \param Isect
	*    A detailed intersection record, which will be filled by the
	*    intersection query
	*
	* \param bShadowRay
	*    \c true if this is a shadow ray query, i.e. a query that only aims to
	*    find out whether the ray is blocked or not without returning detailed
	*    intersection information.
	*
	* \return \c true if an intersection was found
	*/
	bool RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const;

private:
	Mesh * m_pMesh = nullptr;
	BoundingBox3f m_BBox;
};

NAMESPACE_END