#include <integrator\WhittedIntegrator.hpp>
#include <core\Mesh.hpp>
#include <core\Scene.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(WhittedIntegrator, XML_INTEGRATOR_WHITTED);

WhittedIntegrator::WhittedIntegrator(const PropertyList & PropList)
{
	m_Depth = uint32_t(PropList.GetInteger(XML_INTEGRATOR_WHITTED_DEPTH, DEFAULT_INTEGRATOR_WHITTED_DEPTH));
}

Color3f WhittedIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return LiRecursive(pScene, pSampler, Ray, 0);
}

std::string WhittedIntegrator::ToString() const
{
	return tfm::format("WhittedIntegrator[depth = %d]", m_Depth);
}

Color3f WhittedIntegrator::LiRecursive(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray, uint32_t Depth) const
{
	const Emitter * pEnvironmentEmitter = pScene->GetEnvironmentEmitter();
	Color3f Background = pScene->GetBackground();
	bool bForceBackground = pScene->GetForceBackground();

	/* Find the surface that is visible in the requested direction */
	Intersection Isect;
	if (!pScene->RayIntersect(Ray, Isect))
	{
		if (pEnvironmentEmitter != nullptr && !bForceBackground)
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = Ray.Origin;
			EmitterRecord.Wi = Ray.Direction;
			return pEnvironmentEmitter->Eval(EmitterRecord);
		}
		else
		{
			if (Depth == 0)
			{
				return Background;
			}
			else
			{
				return Color3f(0.0f);
			}
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
				BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * Ray.Direction), Isect.ToLocal(Isect.ShadingFrame.N), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, Isect);
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

				if (pEmitter->GetEmitterType() == EEmitterType::EEnvironment || pEmitter->GetEmitterType() == EEmitterType::EDirectional)
				{
					EmitterRecord.Distance = pScene->GetBoundingBox().GetRadius();
				}

				Color3f Li = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
				Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);

				if (!pScene->ShadowRayIntersect(ShadowRay))
				{
					BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * Ray.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, Isect);
					Lr += pBSDF->Eval(BSDFRecord) * std::abs(Frame::CosTheta(BSDFRecord.Wo)) * Li;
				}
			}
		}
	}
	else
	{
		if (pSampler->Next1D() < 0.95f && Depth < m_Depth)
		{
			constexpr float Inv = 1.0f / 0.95f;
			BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * Ray.Direction), ETransportMode::ERadiance, pSampler, Isect);
			Color3f C = pBSDF->Sample(BSDFRecord, pSampler->Next2D());
			Lr += C * LiRecursive(pScene, pSampler, Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo)), Depth + 1) * Inv;
		}
	}

	return Lr + Le;
}

NAMESPACE_END