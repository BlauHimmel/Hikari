#include <integrator\PathMATSIntegrator.hpp>
#include <core\Scene.hpp>
#include <core\Mesh.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PathMATSIntegrator, XML_INTEGRATOR_PATH_MATS);

PathMATSIntegrator::PathMATSIntegrator(const PropertyList & PropList)
{
	m_Depth = uint32_t(PropList.GetInteger(XML_INTEGRATOR_PATH_MATS_DEPTH));

	LOG(WARNING) << "PathMATSIntegrator will emit PointLight and DirectionalLight due to the limitations of the sampling strategy.";
}

Color3f PathMATSIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	Intersection Isect;
	Ray3f TracingRay(Ray);
	Color3f Li(0.0f);
	Color3f Beta(1.0f);
	uint32_t Depth = 0;
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

		if (Isect.pShape->IsEmitter())
		{
			EmitterQueryRecord EmitterRecord;
			EmitterRecord.Ref = TracingRay.Origin;
			EmitterRecord.P = Isect.P;
			EmitterRecord.N = Isect.ShadingFrame.N;
			EmitterRecord.Wi = TracingRay.Direction;
			Color3f Le = Isect.pEmitter->Eval(EmitterRecord);
			Li += Le * Beta;
		}

		const BSDF * pBSDF = Isect.pBSDF;
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

		TracingRay = Ray3f(Isect.P, Isect.ToWorld(BSDFRecord.Wo));
		Depth++;
	}

	return Li;
}

std::string PathMATSIntegrator::ToString() const
{
	return tfm::format("PathMATSIntegrator[depth = %u]", m_Depth);
}

NAMESPACE_END