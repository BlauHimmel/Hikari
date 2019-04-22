#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <initializer_list>

NAMESPACE_BEGIN

/**
* \brief Convenience data structure used to pass multiple
* parameters to the evaluation and sampling routines in \ref BSDF
*/
struct BSDFQueryRecord
{
	/// Incident direction (in the local frame) Starts from the intersection point
	Vector3f Wi;

	/// Outgoing direction (in the local frame) Starts from the intersection point
	Vector3f Wo;

	/// Relative refractive index in the sampled direction
	float Eta;

	/// Measure associated with the sample
	EMeasure Measure;

	/// The transport mode when sampling or evaluating a scattering function
	ETransportMode Mode;

	/// The sampler currently used
	Sampler * pSampler;

	/// Reference to the underlying surface interaction
	const Intersection & Isect;

	/// Create a new record for sampling the BSDF
	BSDFQueryRecord(const Vector3f & Wi, ETransportMode Mode, Sampler * pSampler, const Intersection & Isect);

	/// Create a new record for querying the BSDF
	BSDFQueryRecord(const Vector3f & Wi, const Vector3f & Wo, EMeasure Measure, ETransportMode Mode, Sampler * pSampler, const Intersection & Isect);

	/// Return a human-readable string summary
	std::string ToString() const;
};

/**
* \brief Superclass of all bidirectional scattering distribution functions
*/
class BSDF : public Object
{
public:
	/**
	* \brief Sample the BSDF and return the importance weight (i.e. the
	* value of the BSDF * Cos(Theta_o) divided by the probability density
	* of the sample with respect to solid angles).
	*
	* \param Record  A BSDF query record (Wi is needed)
	* \param Sample  A uniformly distributed sample on \f$[0,1]^2\f$
	*
	* \return The BSDF value divided by the probability density of the sample
	*         sample. The returned value also includes the cosine
	*         foreshortening factor associated with the outgoing direction,
	*         when this is appropriate. A zero value means that sampling
	*         failed.
	*/
	virtual Color3f Sample(BSDFQueryRecord & Record, const Point2f & Sample) const = 0;


	/**
	* \brief Evaluate the BSDF for a pair of directions and measure
	* specified in \code Record
	*
	* \param Record
	*     A record with detailed information on the BSDF query
	* \return
	*     The BSDF value, evaluated for each color channel
	*/
	virtual Color3f Eval(const BSDFQueryRecord & Record) const = 0;

	/**
	* \brief Compute the probability of sampling \c Record.Wo
	* (conditioned on \c Record.Wi).
	*
	* This method provides access to the probability density that
	* is realized by the \ref Sample() method.
	*
	* \param Record
	*     A record with detailed information on the BSDF query
	*
	* \return
	*     A probability/density value expressed with respect
	*     to the specified measure
	*/
	virtual float Pdf(const BSDFQueryRecord & Record) const = 0;

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;

	/**
	* \brief Return whether or not this BRDF is diffuse. This
	* is primarily used by photon mapping to decide whether
	* or not to store photons on a surface
	*/
	virtual bool IsDiffuse() const;

	/**
	* \brief Return whether or not this BRDF is anisotropic. 
	*/
	virtual bool IsAnisotropic() const;

	void AddBSDFType(uint32_t Type);

	bool HasBSDFType(EBSDFType Type) const;

	uint32_t GetBSDFTypes() const;

private:
	uint32_t m_CombinedType = 0;
};

NAMESPACE_END