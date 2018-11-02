#pragma once

#include <core\Common.hpp>
#include <core\Sampler.hpp>
#include <core\Block.hpp>
#include <pcg32.h>

NAMESPACE_BEGIN

/**
* Independent sampling - returns independent uniformly distributed
* random numbers on <tt>[0, 1)x[0, 1)</tt>.
*
* This class is essentially just a wrapper around the pcg32 pseudorandom
* number generator. For more details on what sample generators do in
* general, refer to the \ref Sampler class.
*/
class IndependentSampler : public Sampler
{
public:
	IndependentSampler(const PropertyList & PropList);

	virtual std::unique_ptr<Sampler> Clone() const override;

	virtual void Prepare(const ImageBlock & Block) override;

	virtual void Generate() override;

	virtual void Advance() override;

	virtual float Next1D() override;

	virtual Point2f Next2D() override;

	virtual std::string ToString() const override;

protected:
	IndependentSampler();

	pcg32 m_Random;
};

NAMESPACE_END