#include <integrator\PathEMSIntegrator.hpp>
#include <core\Scene.hpp>
#include <core\Mesh.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathEMSIntegrator, XML_INTEGRATOR_PATH_EMS);

PathEMSIntegrator::PathEMSIntegrator(const PropertyList & PropList)
{
	m_Depth = uint32_t(PropList.GetInteger(XML_INTEGRATOR_PATH_EMS_DEPTH));
}

Color3f PathEMSIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	Intersection Isect;
	Ray3f TracingRay(Ray);
	Color3f Li(0.0f);
	Color3f Beta(1.0f);
	uint32_t Depth = 0;
	bool bLastPathSpecular = false;
	const Emitter * pLastEmitter = nullptr;
	const Emitter * pEnvironmentEmitter = pScene->GetEnvironmentEmitter();
	Color3f Background = pScene->GetBackground();
	bool bForceBackground = pScene->GetForceBackground();

	while (Depth < m_Depth)
	{
		if (!pScene->RayIntersect(TracingRay, Isect))
		{
			if (pEnvironmentEmitter != nullptr && !bForceBackground)
			{
				EmitterQueryRecord EmitterRecord;
				EmitterRecord.Ref = TracingRay.Origin;
				EmitterRecord.Wi = TracingRay.Direction;
				Li += Beta * pEnvironmentEmitter->Eval(EmitterRecord) / 1.0f;
			}
			else if (Depth == 0)
			{
				return Background;
			}
			break;
		}

		Color3f Le(0.0f);

		// Only the first ray from the camera or the ray from the specular reflection
		// /refraction need to account for the emmiter term. In other cases, it has 
		// been computed during the direct light computing part.
		// There also exists a special case such that the ray hit the emissive object
		// continuously.
		if (Isect.pShape->IsEmitter() && (Depth == 0 || bLastPathSpecular || pLastEmitter == Isect.pEmitter))
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = TracingRay.Origin;
			EmitterRecord.P = Isect.P;
			EmitterRecord.N = Isect.ShadingFrame.N;
			EmitterRecord.Wi = TracingRay.Direction;
			Le = Isect.pEmitter->Eval(EmitterRecord);
			Li += Beta * Le;
		}

		const BSDF * pBSDF = Isect.pBSDF;

		if (pBSDF->IsDiffuse())
		{
			bLastPathSpecular = false;

			for (Emitter * pEmitter : pScene->GetEmitters())
			{
				if (pEmitter == Isect.pEmitter)
				{
					continue;
				}

				EmitterQueryRecord EmitterRecord;
				EmitterRecord.Ref = Isect.P;

				if (pEmitter->GetEmitterType() == EEmitterType::EEnvironment || pEmitter->GetEmitterType() == EEmitterType::EDirectional)
				{
					EmitterRecord.Distance = pScene->GetBoundingBox().GetRadius();
				}

				Color3f Ldirect = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
				if (!Ldirect.isZero())
				{
					Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);
					if (!pScene->ShadowRayIntersect(ShadowRay))
					{
						BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * TracingRay.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, Isect);
						Li += Beta * pBSDF->Eval(BSDFRecord) * std::abs(Frame::CosTheta(BSDFRecord.Wo)) * Ldirect;
					}
				}
			}
		}
		else
		{
			bLastPathSpecular = true;
		}

		BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction), ETransportMode::ERadiance, pSampler, Isect);
		Beta *= pBSDF->Sample(BSDFRecord, pSampler->Next2D());

		if (Beta.isZero())
		{
			break;
		}

		// Russian roulette
		if (pSampler->Next1D() < 0.95f)
		{
			constexpr float Inv = 1.0f / 0.95f;
			Beta *= Inv;
		}
		else
		{
			break;
		}

		pLastEmitter = Isect.pEmitter;
		TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		Depth++;
	}

	return Li;
}

std::string PathEMSIntegrator::ToString() const
{
	return tfm::format("PathEMSIntegrator[depth = %u]", m_Depth);
}

NAMESPACE_END