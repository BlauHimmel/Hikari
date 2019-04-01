#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* \brief Statistical test for validating that an importance sampling routine
* (e.g. from a BSDF) produces a distribution that agrees with what the
* implementation claims via its associated density function.
*/
class Chi2Test : public Object
{
public:
	Chi2Test(const PropertyList & PropList);

	virtual ~Chi2Test();

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

	virtual EClassType GetClassType() const override;

protected:
	int m_CosThetaResolution;
	int m_PhiResolution;
	int m_MinExpFrequency;
	int m_SampleCount;
	int m_TestCount;
	float m_SignificanceLevel;
	std::vector<BSDF*> m_pBSDFs;
};

NAMESPACE_END