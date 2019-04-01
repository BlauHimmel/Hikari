#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* Student's t-test for the equality of means
*
* This test analyzes whether the expected value of a random variable matches a
* certain known value. When there is significant statistical "evidence"
* against this hypothesis, the test fails.
*
* This is useful in checking whether a Monte Carlo method method converges
* against the right value. Because statistical tests are able to handle the
* inherent noise of these methods, they can be used to construct statistical
* test suites not unlike the traditional unit tests used in software engineering.
*
* This implementation can be used to test two things:
*
* 1. that the illumination scattered by a BRDF model under uniform illumination
*    into a certain direction matches a given value (modulo noise).
*
* 2. that the average radiance received by a camera within some scene
*    matches a given value (modulo noise).
*/
class StudentTTest : public Object
{
public:
	StudentTTest(const PropertyList & PropList);

	virtual ~StudentTTest();

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual void Activate() override;

	virtual std::string ToString() const override;

	virtual EClassType GetClassType() const override;

protected:
	std::vector<BSDF*> m_pBSDFs;
	std::vector<Scene*> m_pScenes;
	std::vector<float> m_Angles;
	std::vector<float> m_References;
	float m_SignificanceLevel;
	int m_SampleCount;
};

NAMESPACE_END