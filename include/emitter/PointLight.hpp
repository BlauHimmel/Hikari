#pragma once

#include <core\Emitter.hpp>
#include <core\Common.hpp>

NAMESPACE_BEGIN

class PointLight : public Emitter
{
public:
	PointLight(const PropertyList & PropList);

	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

	virtual float Pdf(const EmitterQueryRecord & Record) const override;

	virtual Color3f Eval(const EmitterQueryRecord & Record) const override;

	virtual std::string ToString() const;

protected:
	Color3f m_Power;
	Point3f m_Position;
};

NAMESPACE_END