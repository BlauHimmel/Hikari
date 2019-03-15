#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* \brief Convenience data structure used to pass multiple
* parameters to the evaluation and sampling routines in \ref Emitter
*/
struct EmitterQueryRecord
{
	/// Pointer to the sampled emitter
	const Emitter * pEmitter = nullptr;

	/// Origin point from which we sample the emitter
	Point3f Ref;

	/// Sampled position on the light source
	Point3f P;

	/// Associated surface normal
	Normal3f N;

	/// Solid angle density with respect to Ref
	float Pdf;

	/// Direction vector from 'Ref' to 'P'
	Vector3f Wi;

	/// Distance between 'Ref' and 'P'. 
	/// When the type of emitter is either Env or Directional, this 
	/// value should be set as the radius of the bounding sphere of 
	/// the whole scene before CALLING Sample().
	float Distance;

	/// Create an unitialized query record
	EmitterQueryRecord();

	/// Create a new query record that can be used to sample a emitter
	EmitterQueryRecord(const Point3f & Ref);

	/**
	* \brief Create a query record that can be used to query the
	* sampling density after having intersected an area emitter
	*/
	EmitterQueryRecord(const Emitter * pEmitter, const Point3f & Ref, const Point3f & P, const Normal3f & N);

	/// Return a human-readable string summary
	std::string ToString() const;
};

/**
* \brief Superclass of all emitters
* It is assumed that the visibility test has been done by intergrator.
*/
class Emitter : public Object
{
public:

	/**
	* \brief Sample the emitter and return the importance weight (i.e. the
	* value of the Emitter divided by the probability density
	* of the sample with respect to solid angles).
	*
	* \param Record    An emitter query record (only Ref is needed except for 
	*                  infinity emitter such as EnvironmentLight. In such cases
	*                  Distance is needed as the radius of bounding sphere of the
	*                  entire scene.)
	* \param Sample2D  A uniformly distributed sample on \f$[0,1]^2\f$
	* \param Sample1D  Another optional sample that might be used in some scenarios.
	*
	* \return The emitter value divided by the probability density of the sample.
	*         A zero value means that sampling failed.
	*/
	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const = 0;

	/**
	* \brief Compute the probability of sampling \c Record.P.
	*
	* This method provides access to the probability density that
	* is realized by the \ref Sample() method.
	*
	* \param Record
	*     A record with detailed information on the emitter query.
	*     In most cases, Wi is needed. N and Distance are need for
	*     the AreaLight. 
	*
	* \return
	*     A probability/density value
	*/
	virtual float Pdf(const EmitterQueryRecord & Record) const = 0;

	/**
	* \brief Evaluate the emitter
	*
	* \param Record
	*     A record with detailed information on the emitter query.
	*     In most cases, Wi is needed. N is needed for AreaLight.
	* \return
	*     The emitter value, evaluated for each color channel
	*/
	virtual Color3f Eval(const EmitterQueryRecord & Record) const = 0;

	/**
	* \brief Return the type of object (i.e. Mesh/Emitter/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;

	/**
	* \brief Set the mesh if the emitter is attached to a mesh
	* */
	void SetMesh(Mesh * pMesh);

	EEmitterType GetEmitterType() const;

	bool IsDelta() const;

protected:
	Mesh * m_pMesh = nullptr;
	EEmitterType m_Type = EEmitterType::EUnknown;
};

NAMESPACE_END