#include <integrator\AoIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>
#include <core\Frame.hpp>
#include <core\Sampler.hpp>
#include <core\Sampling.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(AoIntegrator, XML_INTEGRATOR_AO);

AoIntegrator::AoIntegrator(const PropertyList & PropList)
{
	m_Alpha = PropList.GetFloat(XML_INTEGRATOR_AO_ALPHA, DEFAULT_INTEGRATOR_AO_ALPHA);
	m_SampleCount = uint32_t(PropList.GetInteger(XML_INTEGRATOR_AO_SAMPLE_COUNT, DEFAULT_INTEGRATOR_AO_SAMPLE_COUNT));
	m_InvSampleCount = 1.0f / float(m_SampleCount);
}

Color3f AoIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		return Color3f(0.0f);
	}

	Color3f Li(0.0f);

	for (uint32_t i = 0; i < m_SampleCount; i++)
	{
		Vector3f W = Sampling::SquareToCosineHemisphere(pSampler->Next2D());
		Vector3f Pt = Isect.P + m_Alpha * Isect.ShadingFrame.ToWorld(W);

		Ray3f AoRay;
		AoRay.Origin = Isect.P + Isect.GeometricFrame.N * float(Epsilon);
		AoRay.Direction = Pt - AoRay.Origin;
		AoRay.MaxT = 1.0f;
		AoRay.MinT = 0.0f;
		AoRay.Update();

		if (!pScene->ShadowRayIntersect(AoRay))
		{
			Li += Color3f(1.0f);
		}
	}

	return Li * m_InvSampleCount;
}

std::string AoIntegrator::ToString() const
{
	return tfm::format(
		"AoIntegrator[alpha = %.4f, sampleCount = %d]",
		m_Alpha,
		m_SampleCount
	);
}

NAMESPACE_END