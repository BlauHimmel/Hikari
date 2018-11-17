#include <integrator\WhittedIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(WhittedIntegrator, XML_INTEGRATOR_WHITTED);

WhittedIntegrator::WhittedIntegrator(const PropertyList & PropList)
{

}

Color3f WhittedIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		return Color3f(0.0f);
	}

	Color3f Le(0.0f);
	if (Isect.pMesh->IsEmitter())
	{
		EmitterQueryRecord EmitterRecord;
		EmitterRecord.N = Isect.ShadingFrame.N;
		EmitterRecord.Wi = Ray.Direction;
		Le = Isect.pMesh->GetEmitter()->Eval(EmitterRecord);
	}

	Color3f Lr(0.0f);
	const BSDF * pBSDF = Isect.pMesh->GetBSDF();

	for (Emitter * pEmitter : pScene->GetEmitters())
	{
		EmitterQueryRecord EmitterRecord;
		EmitterRecord.Ref = Isect.P;

		Color3f Li = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
		Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);

		if (!pScene->ShadowRayIntersect(ShadowRay))
		{
			BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * Ray.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle);
			float G = std::abs(
				EmitterRecord.Wi.dot(Isect.ShadingFrame.N) * (-1.0f * EmitterRecord.Wi).dot(EmitterRecord.N) / 
				(EmitterRecord.Distance * EmitterRecord.Distance)
			);

			Lr += pBSDF->Eval(BSDFRecord) * Li * G;
		}
	}

	return Lr + Le;
}

std::string WhittedIntegrator::ToString() const
{
	return "WhittedIntegrator[]";
}

NAMESPACE_END