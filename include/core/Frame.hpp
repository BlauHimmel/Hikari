#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>

NAMESPACE_BEGIN

/**
* \brief Stores a three-dimensional orthonormal coordinate frame
*
* This class is mostly used to quickly convert between different
* cartesian coordinate systems and to efficiently compute certain
* quantities (e.g. \ref CosTheta(), \ref TanTheta, ..).
*/
struct Frame
{
	Vector3f S;
	Vector3f T;
	Normal3f N;

	/// Default constructor -- performs no initialization!
	Frame();

	Frame(const Normal3f & N, const Vector3f & Dpdu);

	/// Given a normal and tangent vectors, construct a new coordinate frame
	Frame(const Vector3f & S, const Vector3f & T, const Normal3f & N);

	/// Construct a frame from the given orthonormal vectors
	Frame(const Vector3f & X, const Vector3f & Y, const Vector3f & Z);

	/// Construct a new coordinate frame from a single vector
	Frame(const Vector3f & N);

	/// Convert from world coordinates to local coordinates
	Vector3f ToLocal(const Vector3f & V) const;

	/// Convert from local coordinates to world coordinates
	Vector3f ToWorld(const Vector3f & V) const;

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the cosine of the angle between the normal and v */
	static float CosTheta(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the sine of the angle between the normal and v */
	static float SinTheta(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the tangent of the angle between the normal and v */
	static float TanTheta(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the squared sine of the angle between the normal and v */
	static float SinTheta2(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the squared cosine of the angle between the normal and v */
	static float CosTheta2(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the sine of the phi parameter in spherical coordinates */
	static float SinPhi(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the cosine of the phi parameter in spherical coordinates */
	static float CosPhi(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the squared sine of the phi parameter in  spherical
	* coordinates */
	static float SinPhi2(const Vector3f & V);

	/** \brief Assuming that the given direction is in the local coordinate
	* system, return the squared cosine of the phi parameter in  spherical
	* coordinates */
	static float CosPhi2(const Vector3f & V);

	/// Equality test
	bool operator==(const Frame & Rhs) const;

	/// Inequality test
	bool operator!=(const Frame & Rhs) const;

	/// Return a human-readable string summary of this frame
	std::string ToString() const;
};

NAMESPACE_END