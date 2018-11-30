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
	Intersection Isect;
	Ray3f TracingRay(Ray);
	Color3f Li(0.0f);
	Color3f Beta(1.0f);
	uint32_t Depth = 0;
	float WeightEMS = 1.0f, WeightMATS = 1.0f;

	while (Depth < m_Depth)
	{
		if (!pScene->RayIntersect(TracingRay, Isect))
		{
			break;
		}

		float PdfLightEMS = 0.0f, PdfBSDFEMS = 0.0f;
		float PdfLightMATS = 0.0f, PdfBSDFMATS = 0.0f;

		const BSDF * pBSDF = Isect.pBSDF;

		// Only the first ray from the camera or the ray from the specular reflection
		// /refraction need to account for the emmiter term. In other cases, it has 
		// been computed during the direct light computing part.
		// There also exists a special case such that the ray hit the emissive object
		// continuously.
		if (Isect.pShape->IsEmitter())
		{
			EmitterQueryRecord EmitterRecord(Isect.pEmitter, TracingRay.Origin, Isect.P, Isect.ShadingFrame.N);

			Color3f Le = Isect.pEmitter->Eval(EmitterRecord);
			Li += Beta * WeightMATS * Le;
		}

		const std::vector<Emitter*> & pEmitters = pScene->GetEmitters();
		Emitter * pEmitter = pEmitters[size_t((pEmitters.size() - 1) * pSampler->Next1D())];
		EmitterQueryRecord EmitterRecord(Isect.P);

		Color3f Ldirect = float(pEmitters.size()) * pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
		PdfLightEMS = EmitterRecord.Pdf;

		if (!Ldirect.isZero())
		{
			Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);
			if (!pScene->ShadowRayIntersect(ShadowRay))
			{
				BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle);
				PdfBSDFEMS = pBSDF->Pdf(BSDFRecord);
				if (PdfLightEMS + PdfBSDFEMS != 0.0f)
				{
					WeightEMS = PdfLightEMS / (PdfLightEMS + PdfBSDFEMS);
				}
				Li += Beta * pBSDF->Eval(BSDFRecord) * Frame::CosTheta(BSDFRecord.Wo) * Ldirect * WeightEMS;
			}
		}

		BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction));
		Color3f F = pBSDF->Sample(BSDFRecord, pSampler->Next2D());

		TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		Beta *= F;

		if (Beta.isZero())
		{
			break;
		}

		Intersection IsectNext;
		if (pScene->RayIntersect(TracingRay, IsectNext) && IsectNext.pEmitter != nullptr)
		{
			EmitterRecord = EmitterQueryRecord(IsectNext.pEmitter, Isect.P, IsectNext.P, IsectNext.ShadingFrame.N);

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