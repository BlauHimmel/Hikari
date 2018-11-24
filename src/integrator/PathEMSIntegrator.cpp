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
	Color3f C(1.0f);
	uint32_t Depth = 0;

	while (Depth < m_Depth)
	{
		if (!pScene->RayIntersect(TracingRay, Isect))
		{
			break;
		}

		if (Isect.pShape->IsEmitter())
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = TracingRay.Origin;
			EmitterRecord.P = Isect.P;
			EmitterRecord.N = Isect.ShadingFrame.N;
			EmitterRecord.Wi = TracingRay.Direction;
			Color3f Le = Isect.pEmitter->Eval(EmitterRecord);
			Li += Le * C;
		}

		const BSDF * pBSDF = Isect.pBSDF;

		const std::vector<Emitter*> pEmitters = pScene->GetEmitters();
		Emitter * pEmitter = pEmitters[size_t(pSampler->Next1D() * (pEmitters.size() - 1))];

		if (pBSDF->IsDiffuse() && pEmitter != Isect.pEmitter)
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = Isect.P;

			Color3f Ldirect = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
			Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);

			if (!pScene->ShadowRayIntersect(ShadowRay))
			{
				BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * Ray.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle);
				float Pdf = pBSDF->Pdf(BSDFRecord);
				if (Pdf != 0.0f)
				{
					Color3f F = pBSDF->Eval(BSDFRecord) / pBSDF->Pdf(BSDFRecord) * Frame::CosTheta(BSDFRecord.Wo);
					C *= F;
					Li += C * Ldirect;
				}
			}

			if (C.isZero())
			{
				break;
			}

			TracingRay = Ray3f(Isect.P, EmitterRecord.Wi);
		}
		else
		{
			BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction));
			C *= pBSDF->Sample(BSDFRecord, pSampler->Next2D());

			if (C.isZero())
			{
				break;
			}

			TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		}

		// Russian roulette
		if (pSampler->Next1D() < 0.95f)
		{
			constexpr float Inv = 1.0f / 0.95f;
			C *= Inv;
		}
		else
		{
			break;
		}

		Depth++;
	}

	return Li;
}

std::string PathEMSIntegrator::ToString() const
{
	return tfm::format("PathEMSIntegrator[depth = %u]", m_Depth);
}

NAMESPACE_END