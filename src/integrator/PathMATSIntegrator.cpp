#include <integrator\PathMATSIntegrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathMATSIntegrator, XML_INTEGRATOR_PATH_MATS);

PathMATSIntegrator::PathMATSIntegrator(const PropertyList & PropList)
{

}

Color3f PathMATSIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return Color3f();
}

std::string PathMATSIntegrator::ToString() const
{
	return "PathMATSIntegrator[]";
}

NAMESPACE_END