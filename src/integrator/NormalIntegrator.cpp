#include <integrator\NormalIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(NormalIntegrator, XML_INTEGRATOR_NORMAL);

NormalIntegrator::NormalIntegrator(const PropertyList & PropList)
{

}

Color3f NormalIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		return Color3f(0.0f);
	}

	/* Return the component-wise absolute value of the shading normal as a color */
	Normal3f N = Isect.ShadingFrame.N.cwiseAbs();
	return Color3f(N.x(), N.y(), N.z());
}

std::string NormalIntegrator::ToString() const
{
	return tfm::format("NormalIntegrator[]");
}

NAMESPACE_END
