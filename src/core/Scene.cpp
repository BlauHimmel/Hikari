#include <core\Scene.hpp>
#include <core\Sampler.hpp>
#include <core\Camera.hpp>
#include <core\Acceleration.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(Scene, XML_SCENE);

Scene::Scene(const PropertyList & PropList)
{
	m_pAcceleration.reset(new Acceleration());
}

Scene::~Scene()
{
	for (auto pPtr : m_pMeshes)
	{
		delete pPtr;
	}
	m_pMeshes.clear();
}

const Acceleration * Scene::GetAccel() const
{
	return m_pAcceleration.get();
}

const Integrator * Scene::GetIntegrator() const
{
	return m_pIntegrator.get();
}

Integrator * Scene::GetIntegrator()
{
	return m_pIntegrator.get();
}

const Camera * Scene::GetCamera() const
{
	return m_pCamera.get();
}

const Sampler * Scene::GetSampler() const
{
	return m_pSampler.get();
}

Sampler * Scene::GetSampler()
{
	return m_pSampler.get();
}

const std::vector<Mesh*> & Scene::GetMeshes() const
{
	return m_pMeshes;
}

bool Scene::RayIntersect(const Ray3f & Ray, Intersection & Isect) const
{
	return m_pAcceleration->RayIntersect(Ray, Isect, false);
}

void Scene::Activate()
{
	m_pAcceleration->Build();

	if (m_pIntegrator == nullptr)
	{
		throw HikariException("No integrator was specified!");
	}
	if (m_pCamera == nullptr)
	{
		throw HikariException("No camera was specified!");
	}

	if (m_pSampler == nullptr)
	{
		/* Create a default (independent) sampler */
		m_pSampler.reset((Sampler*)(ObjectFactory::CreateInstance("independent", PropertyList())));
	}

	LOG(INFO) << endl;
	LOG(INFO) << "Configuration: " << ToString();
	LOG(INFO) << endl;
}

void Scene::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EMesh:
		m_pAcceleration->AddMesh((Mesh*)(pChildObj));
		m_pMeshes.push_back((Mesh*)(pChildObj));
		break;
	case EClassType::EEmitter:
		throw HikariException("Scene::AddChild(): You need to implement this for emitters");
		break;
	case EClassType::ESampler:
		if (m_pSampler != nullptr)
		{
			throw HikariException("There can only be one sampler per scene!");
		}
		m_pSampler.reset((Sampler*)(pChildObj));
		break;
	case EClassType::ECamera:
		if (m_pCamera)
		{
			throw HikariException("There can only be one camera per scene!");
		}
		m_pCamera.reset((Camera*)(pChildObj));
		break;
	case EClassType::EIntegrator:
		if (m_pIntegrator)
		{
			throw HikariException("There can only be one integrator per scene!");
		}
		m_pIntegrator.reset((Integrator*)(pChildObj));
		break;
	default:
		throw HikariException("Scene::AddChild(<%s>) is not supported!", ClassTypeName(pChildObj->GetClassType()));
	}
}

std::string Scene::ToString() const
{
	std::string MeshesString;
	for (size_t i = 0; i<m_pMeshes.size(); ++i)
	{
		MeshesString += std::string("  ") + Indent(m_pMeshes[i]->ToString(), 2);
		if (i + 1 < m_pMeshes.size())
		{
			MeshesString += ",";
		}
		MeshesString += "\n";
	}

	return tfm::format(
		"Scene[\n"
		"  integrator = %s,\n"
		"  sampler = %s\n"
		"  camera = %s,\n"
		"  meshes = {\n"
		"  %s  }\n"
		"]",
		Indent(m_pIntegrator->ToString()),
		Indent(m_pSampler->ToString()),
		Indent(m_pCamera->ToString()),
		Indent(MeshesString, 2)
	);
}

Object::EClassType Scene::GetClassType() const
{
	return EClassType::EScene;
}

NAMESPACE_END
