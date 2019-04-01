#include <integrator\PathMISIntegrator.hpp>
#include <core\Scene.hpp>
#include <core\Mesh.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathMISIntegrator, XML_INTEGRATOR_PATH_MIS);

PathMISIntegrator::PathMISIntegrator(const PropertyList & PropList)
{
	m_Depth = uint32_t(PropList.GetInteger(XML_INTEGRATOR_PATH_MIS_DEPTH));
}

Color3f PathMISIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	Intersection IsectNext;
	bool bFoundIntersectionNext = false;

	Intersection Isect;
	Ray3f TracingRay(Ray);
	Color3f Li(0.0f);
	Color3f Beta(1.0f);
	uint32_t Depth = 0;
	float WeightEMS = 1.0f, WeightMATS = 1.0f;
	const Emitter * pEnvironmentEmitter = pScene->GetEnvironmentEmitter();
	Color3f Background = pScene->GetBackground();
	bool bForceBackground = pScene->GetForceBackground();

	while (Depth < m_Depth)
	{
		if (Depth == 0)
		{
			if (!pScene->RayIntersect(TracingRay, Isect))
			{
				if (pEnvironmentEmitter != nullptr && !bForceBackground)
				{
					EmitterQueryRecord EmitterRecord;
					EmitterRecord.Ref = TracingRay.Origin;
					EmitterRecord.Wi = TracingRay.Direction;
					Li += Beta * pEnvironmentEmitter->Eval(EmitterRecord) / 1.0f;
					break;
				}
				else
				{
					return Background;
				}
			}
		}
		else
		{
			if (bFoundIntersectionNext)
			{
				Isect = IsectNext;
			}
			else
			{
				if (pEnvironmentEmitter != nullptr)
				{
					EmitterQueryRecord EmitterRecord;
					EmitterRecord.Ref = TracingRay.Origin;
					EmitterRecord.Wi = TracingRay.Direction;
					Li += Beta * pEnvironmentEmitter->Eval(EmitterRecord) / 1.0f;
				}
				break;
			}
		}

		float PdfLightEMS = 0.0f, PdfBSDFEMS = 0.0f;
		float PdfLightMATS = 0.0f, PdfBSDFMATS = 0.0f;

		const BSDF * pBSDF = Isect.pBSDF;

		if (Isect.pShape->IsEmitter())
		{
			EmitterQueryRecord EmitterRecord(Isect.pEmitter, TracingRay.Origin, Isect.P, Isect.ShadingFrame.N);

			Color3f Le = Isect.pEmitter->Eval(EmitterRecord);
			Li += Beta * WeightMATS * Le;
		}

		for (Emitter * pEmitter : pScene->GetEmitters())
		{
			EmitterQueryRecord EmitterRecord(Isect.P);

			if (pEmitter->GetEmitterType() == EEmitterType::EEnvironment || pEmitter->GetEmitterType() == EEmitterType::EDirectional)
			{
				EmitterRecord.Distance = pScene->GetBoundingBox().GetRadius();
			}

			Color3f Ldirect = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
			PdfLightEMS = EmitterRecord.Pdf;

			if (!Ldirect.isZero())
			{
				Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);
				if (!pScene->ShadowRayIntersect(ShadowRay))
				{
					BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, Isect);
					PdfBSDFEMS = pBSDF->Pdf(BSDFRecord);
					if (PdfLightEMS + PdfBSDFEMS != 0.0f)
					{
						WeightEMS = PdfLightEMS / (PdfLightEMS + PdfBSDFEMS);
					}
					Li += Beta * pBSDF->Eval(BSDFRecord) * std::abs(Frame::CosTheta(BSDFRecord.Wo)) * Ldirect * WeightEMS;
				}
			}
		}

		BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction), ETransportMode::ERadiance, pSampler, Isect);
		Color3f F = pBSDF->Sample(BSDFRecord, pSampler->Next2D());

		TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		Beta *= F;

		if (Beta.isZero())
		{
			break;
		}

		bFoundIntersectionNext = pScene->RayIntersect(TracingRay, IsectNext);
		if (bFoundIntersectionNext && IsectNext.pEmitter != nullptr)
		{
			EmitterQueryRecord EmitterRecord(IsectNext.pEmitter, Isect.P, IsectNext.P, IsectNext.ShadingFrame.N);

			PdfLightMATS = IsectNext.pEmitter->Pdf(EmitterRecord);
			PdfBSDFMATS = pBSDF->Pdf(BSDFRecord);

			if (PdfBSDFMATS + PdfLightMATS != 0.0f)
			{
				WeightMATS = PdfBSDFMATS / (PdfBSDFMATS + PdfLightMATS);
			}
		}

		if (BSDFRecord.Measure == EMeasure::EDiscrete)
		{
			WeightEMS = 0.0f;
			WeightMATS = 1.0f;
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

		Depth++;
	}

	return Li;
}

std::string PathMISIntegrator::ToString() const
{
	return tfm::format("PathMISIntegrator[depth = %u]", m_Depth);
}

NAMESPACE_END