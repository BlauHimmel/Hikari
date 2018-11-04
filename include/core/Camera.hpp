#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

/**
* \brief Generic camera interface
*
* This class provides an abstract interface to cameras in Hikari and
* exposes the ability to sample their response function.
*/
class Camera : public Object
{
public:
	/**
	* \brief Importance sample a ray according to the camera's response function
	*
	* \param Ray
	*    A ray data structure to be filled with a position
	*    and direction value
	*
	* \param SamplePosition
	*    Denotes the desired sample position on the film
	*    expressed in fractional pixel coordinates
	*
	* \param ApertureSample
	*    A uniformly distributed 2D vector that is used to sample
	*    a position on the aperture of the sensor if necessary.
	*
	* \return
	*    An importance weight associated with the sampled ray.
	*    This accounts for the difference in the camera response
	*    function and the sampling density.
	*/
	virtual Color3f SampleRay(Ray3f & Ray, const Point2f & SamplePosition, const Point2f & ApertureSample) const = 0;

	/// Return the size of the output image in pixels
	const Vector2i & GetOutputSize() const;

	/// Return the camera's reconstruction filter in image space
	const ReconstructionFilter * GetReconstructionFilter() const;

	/**
	* \brief Return the type of object (i.e. Mesh/Camera/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;

protected:
	Vector2i m_OutputSize;
	ReconstructionFilter * m_pFilter;
};

NAMESPACE_END