#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

/**
* \brief Intersection data structure
*
* This data structure records local information about a intersection.
* This includes the position, traveled ray distance, uv coordinates, as well
* as well as two local coordinate frames (one that corresponds to the true
* geometry, and one that is used for shading computations).
*/
struct Intersection
{
	/// Position of the surface intersection
	Point3f P;

	/// Unoccluded distance along the ray
	float T = std::numeric_limits<float>::max();

	/** \brief UV coordinates, if any
	* (This parameter was used as barycentric coordinate in the 1st stage
	* of ray-triangle intersection and was transformed to UV coordinate in
	* the 2nd stage)
	*/
	Point2f UV;

	bool bHasUVPartial = false;

	Vector3f dPdU;

	Vector3f dPdV;

	Vector3f dNdU;

	Vector3f dNdV;

	float dUdX = 0.0f;

	float dUdY = 0.0f;

	float dVdX = 0.0f;

	float dVdY = 0.0f;

	/// Shading frame (based on the shading normal)
	Frame ShadingFrame;

	/// Geometric frame (based on the true geometry)
	Frame GeometricFrame;

	/// Pointer to the associated mesh
	const Shape * pShape = nullptr;

	/// Pointer to the associated Emitter (nullptr if not a emitter)
	const Emitter * pEmitter = nullptr;

	/// Pointer to the associated BSDF
	const BSDF * pBSDF = nullptr;

	/// Create an uninitialized intersection record
	Intersection();

	/// Transform a direction vector into the local shading frame
	Vector3f ToLocal(const Vector3f & Dir) const;

	/// Transform a direction vector from local to world coordinates
	Vector3f ToWorld(const Vector3f & Dir) const;

	/** \brief Spawn a shadow ray from the intersection to the given point
	* Note : MinT = 0.0 Max : 1.0 and Dir = Pt - Isect.P in the returned ray
	*/
	Ray3f SpawnShadowRay(const Point3f & Pt) const;

	/// Computes texture coordinate partials
	void ComputeScreenSpacePartial(const Ray3f & Ray);

	/// Return a human-readable summary of the intersection record
	std::string ToString() const;
};

NAMESPACE_END