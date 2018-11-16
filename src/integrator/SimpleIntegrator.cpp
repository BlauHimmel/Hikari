#include <integrator\SimpleIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(SimpleIntegrator, XML_INTEGRATOR_SIMPLE);

SimpleIntegrator::SimpleIntegrator(const PropertyList & PropList)
{
	m_Position = PropList.GetPoint(XML_INTEGRATOR_SIMPLE_POSITION);
	m_Energy = PropList.GetColor(XML_INTEGRATOR_SIMPLE_ENERGY);
}

Color3f SimpleIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return Color3f();
}

std::string SimpleIntegrator::ToString() const
{
	return std::string();
}

NAMESPACE_END