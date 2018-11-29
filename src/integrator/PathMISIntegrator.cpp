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
	bool bLastPathSpecular = false;
	const Emitter * pLastEmitter = nullptr;

	while (Depth < m_Depth)
	{
		if (!pScene->RayIntersect(TracingRay, Isect))
		{
			break;
		}

		float PdfLight = 0.5f, PdfBSDF = 0.5f;

		const BSDF * pBSDF = Isect.pBSDF;

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

				Color3f Ldirect = pEmitter->Sample(EmitterRecord, pSampler->Next2D(), pSampler->Next1D());
				PdfLight = EmitterRecord.Pdf;

				Ray3f ShadowRay = Isect.SpawnShadowRay(EmitterRecord.P);

				if (!pScene->ShadowRayIntersect(ShadowRay))
				{
					BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0 * TracingRay.Direction), Isect.ToLocal(EmitterRecord.Wi), EMeasure::ESolidAngle);
					PdfBSDF = pBSDF->Pdf(BSDFRecord);
					Li += Beta * pBSDF->Eval(BSDFRecord) * Frame::CosTheta(BSDFRecord.Wo) * Ldirect * (PdfLight / (PdfLight + PdfBSDF));
				}
			}
		}
		else
		{
			bLastPathSpecular = true;
		}

		BSDFQueryRecord BSDFRecord(Isect.ToLocal(-1.0f * TracingRay.Direction));
		Color3f F = pBSDF->Sample(BSDFRecord, pSampler->Next2D());

		TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		pLastEmitter = Isect.pEmitter;

		if (pScene->RayIntersect(TracingRay, Isect) && Isect.pEmitter != nullptr)
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = TracingRay.Origin;
			EmitterRecord.P = Isect.P;
			EmitterRecord.N = Isect.ShadingFrame.N;
			EmitterRecord.Wi = TracingRay.Direction;
			EmitterRecord.Distance = (EmitterRecord.P - EmitterRecord.Ref).norm();

			Color3f Ldirect = Isect.pEmitter->Eval(EmitterRecord);

			PdfLight = Isect.pEmitter->Pdf(EmitterRecord);

			if (Isect.pBSDF->IsDiffuse())
			{
				PdfBSDF = pBSDF->Pdf(BSDFRecord);
			}
			else
			{
				PdfBSDF = 1.0f;
			}

			Li += Beta * F * Ldirect * (PdfBSDF / (PdfLight + PdfBSDF));
		}

		Beta *= F;

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

		Depth++;
	}

	return Li;
}

std::string PathMISIntegrator::ToString() const
{
	return tfm::format("PathMISIntegrator[depth = %u]", m_Depth);
}

NAMESPACE_END