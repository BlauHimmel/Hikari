#pragma once

#include <core\Emitter.hpp>
#include <core\Common.hpp>

NAMESPACE_BEGIN

class DirectionalLight : public Emitter
{
public:
	DirectionalLight(const PropertyList & PropList);

	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

	virtual float Pdf(const EmitterQueryRecord & Record) const override;

	virtual Color3f Eval(const EmitterQueryRecord & Record) const override;

	virtual std::string ToString() const;

protected:
	Color3f m_Irradiance;
	Vector3f m_Direction;
};

NAMESPACE_END