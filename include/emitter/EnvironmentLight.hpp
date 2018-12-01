#pragma once

#include <core\Common.hpp>
#include <core\Emitter.hpp>
#include <core\Transform.hpp>

NAMESPACE_BEGIN

class EnvironmentLight : public Emitter
{
public:
	EnvironmentLight(const PropertyList & PropList);

	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

	virtual float Pdf(const EmitterQueryRecord & Record) const override;

	virtual Color3f Eval(const EmitterQueryRecord & Record) const override;

	virtual std::string ToString() const;

protected:
	std::string m_Name;
	float m_Scale;
	Transform m_ToWorld;
};

NAMESPACE_END