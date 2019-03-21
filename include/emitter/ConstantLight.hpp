#pragma once

#include <core\Emitter.hpp>
#include <core\Common.hpp>

NAMESPACE_BEGIN

/* A constant enviornment emitter which often used for the basic BSDF, such as diffuse. */
class ConstantLight : public Emitter
{
public:
	ConstantLight(const PropertyList & PropList);

	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

	virtual float Pdf(const EmitterQueryRecord & Record) const override;

	virtual Color3f Eval(const EmitterQueryRecord & Record) const override;

	virtual std::string ToString() const;

protected:
	Color3f m_Radiance;
};

NAMESPACE_END