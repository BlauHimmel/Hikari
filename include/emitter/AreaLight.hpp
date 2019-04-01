#pragma once

#include <core\Emitter.hpp>
#include <core\Common.hpp>

NAMESPACE_BEGIN

class AreaLight : public Emitter
{
public:
	AreaLight(const PropertyList & PropList);

	virtual Color3f Sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

	virtual float Pdf(const EmitterQueryRecord & Record) const override;

	virtual Color3f Eval(const EmitterQueryRecord & Record) const override;

	virtual void SetParent(Object * pParentObj, const std::string & Name) override;
	
	virtual std::string ToString() const;

protected:
	Color3f m_Radiance;
};

NAMESPACE_END