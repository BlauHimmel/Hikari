#include <integrator\PathMISIntegrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathMISIntegrator, XML_INTEGRATOR_PATH_MIS);

PathMISIntegrator::PathMISIntegrator(const PropertyList & PropList)
{

}

Color3f PathMISIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return Color3f();
}

std::string PathMISIntegrator::ToString() const
{
	return "PathMISIntegrator[]";
}

NAMESPACE_END