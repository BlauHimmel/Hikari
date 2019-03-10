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
	const Emitter * pEnvironmentEmitter = pScene->GetEnvironmentEmitter();

	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		if (pEnvironmentEmitter != nullptr)
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = Ray.Origin;
			EmitterRecord.Wi = Ray.Direction;
			return pEnvironmentEmitter->Eval(EmitterRecord);
		}
		else
		{
			return Color3f(0.0f);
		}
	}

	Color3f Le(0.0f);
	if (Isect.pShape->IsEmitter())
	{
		EmitterQueryRecord EmitterRecord;
		EmitterRecord.Ref = Ray.Origin;
		EmitterRecord.P = Isect.P;
		EmitterRecord.N = Isect.ShadingFrame.N;
		EmitterRecord.Wi = Ray.Direction;
		Le = Isect.pEmitter->Eval(EmitterRecord);
	}

	Color3f Lr(0.0f);
	const BSDF * pBSDF = Isect.pBSDF;

	if (pBSDF->IsDiffuse())
	{
		for (Emitter * pEmitter : pScene->GetEmitters())
		{
			if (pEmitter == Isect.pEmitter)
			{
				BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * Ray.Direction), Isect.ToLocal(Isect.ShadingFrame.N), EMeasure::ESolidAngle, ETransportMode::ERadiance, nullptr);
				float Pdf = pBSDF->Pdf(BSDFRecord);
				if (Pdf != 0.0f)
				{
					Lr += pBSDF->Eval(BSDFRecord) / Pdf * std::abs(Frame::CosTheta(BSDFRecord.Wo)) * Le;
				}
				continue;
			}
			else
			{
				EmitterQueryRecord EmitterRecord;
				EmitterRecord.Ref = Isect.P;

				if (pEmitter->GetEmitterType() == EEmitterType::EEnvironment)
				{
					EmitterRecord.Distance = pScene->GetBoundingBox().GetRadius();
				}

				Color3f Li = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
				Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);

				if (!pScene->ShadowRayIntersect(ShadowRay))
				{
					BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * Ray.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, nullptr);
					Lr += pBSDF->Eval(BSDFRecord) * std::abs(Frame::CosTheta(BSDFRecord.Wo)) * Li;
				}
			}
		}
	}
	else
	{
		if (pSampler->Next1D() < 0.95f)
		{
			constexpr float Inv = 1.0f / 0.95f;
			BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * Ray.Direction), ETransportMode::ERadiance, nullptr);
			Color3f C = pBSDF->Sample(BSDFRecord, pSampler->Next2D());
			Lr += C * Li(pScene, pSampler, Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo))) * Inv;
		}
	}

	return Lr + Le;
}

std::string WhittedIntegrator::ToString() const
{
	return "WhittedIntegrator[]";
}

NAMESPACE_END