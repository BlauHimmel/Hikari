#include <integrator\PathEMSIntegrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathEMSIntegrator, XML_INTEGRATOR_PATH_EMS);

PathEMSIntegrator::PathEMSIntegrator(const PropertyList & PropList)
{
	m_Depth = uint32_t(PropList.GetInteger(XML_INTEGRATOR_PATH_EMS_DEPTH));
}

Color3f PathEMSIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return Color3f();
}

std::string PathEMSIntegrator::ToString() const
{
	return "PathEMSIntegrator[]";
}

NAMESPACE_END