#include <core\Scene.hpp>
#include <core\Sampler.hpp>
#include <core\Camera.hpp>
#include <core\Acceleration.hpp>
#include <core\Integrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(Scene, XML_SCENE);

Scene::Scene(const PropertyList & PropList)
{

}

Scene::~Scene()
{
	for (auto pPtr : m_pMeshes)
	{
		delete pPtr;
	}
	m_pMeshes.clear();
	m_pMeshes.shrink_to_fit();

	for (auto pPtr : m_pEmitters)
	{
		delete pPtr;
	}
	m_pEmitters.clear();
	m_pEmitters.shrink_to_fit();

	delete m_pAcceleration;
	delete m_pSampler;
	delete m_pCamera;
	delete m_pIntegrator;
}

const Acceleration * Scene::GetAccel() const
{
	return m_pAcceleration;
}

const Integrator * Scene::GetIntegrator() const
{
	return m_pIntegrator;
}

Integrator * Scene::GetIntegrator()
{
	return m_pIntegrator;
}

const Camera * Scene::GetCamera() const
{
	return m_pCamera;
}

const Sampler * Scene::GetSampler() const
{
	return m_pSampler;
}

Sampler * Scene::GetSampler()
{
	return m_pSampler;
}

const std::vector<Mesh*> & Scene::GetMeshes() const
{
	return m_pMeshes;
}

const std::vector<Emitter*> & Scene::GetEmitters() const
{
	return m_pEmitters;
}

bool Scene::RayIntersect(const Ray3f & Ray, Intersection & Isect) const
{
	return m_pAcceleration->RayIntersect(Ray, Isect, false);
}

bool Scene::ShadowRayIntersect(const Ray3f & Ray) const
{
	Intersection Isect;
	return m_pAcceleration->RayIntersect(Ray, Isect, true);
}

void Scene::Activate()
{
	if (m_pAcceleration == nullptr)
	{
		/* Create a default acceleration */
		LOG(WARNING) << "No acceleration was specified, create a default acceleration.";
		m_pAcceleration = (Acceleration*)(ObjectFactory::CreateInstance(DEFAULT_SCENE_ACCELERATION, PropertyList()));
	}

	for (Mesh * pMesh : m_pMeshes)
	{
		m_pAcceleration->AddMesh(pMesh);
	}

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
		/* Create a default sampler */
		LOG(WARNING) << "No sampler was specified, create a default sampler.";
		m_pSampler = (Sampler*)(ObjectFactory::CreateInstance(DEFAULT_SCENE_SAMPLER, PropertyList()));
	}

	LOG(INFO) << "\nConfiguration:\n" << ToString();
}

void Scene::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EAcceleration:
		if (m_pAcceleration != nullptr)
		{
			throw HikariException("There can only be one acceleration per scene!");
		}
		m_pAcceleration = (Acceleration*)(pChildObj);
		break;
	case EClassType::EMesh:
		m_pMeshes.push_back((Mesh*)(pChildObj));
		if (((Mesh*)(pChildObj))->IsEmitter())
		{
			m_pEmitters.push_back(((Mesh*)(pChildObj))->GetEmitter());
		}
		break;
	case EClassType::EEmitter:
		if (((Emitter*)(pChildObj))->GetEmitterType() == EEmitterType::EPoint ||
			((Emitter*)(pChildObj))->GetEmitterType() == EEmitterType::EEnvironment)
		{
			m_pEmitters.push_back((Emitter*)(pChildObj));
		}
		else
		{
			throw HikariException("Scene::AddChild(): You need to implement this for emitters");
		}
		break;
	case EClassType::ESampler:
		if (m_pSampler != nullptr)
		{
			throw HikariException("There can only be one sampler per scene!");
		}
		m_pSampler = (Sampler*)(pChildObj);
		break;
	case EClassType::ECamera:
		if (m_pCamera)
		{
			throw HikariException("There can only be one camera per scene!");
		}
		m_pCamera = (Camera*)(pChildObj);
		break;
	case EClassType::EIntegrator:
		if (m_pIntegrator)
		{
			throw HikariException("There can only be one integrator per scene!");
		}
		m_pIntegrator = (Integrator*)(pChildObj);
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

	std::string EmitterString;
	for (size_t i = 0; i<m_pEmitters.size(); ++i)
	{
		EmitterString += std::string("  ") + Indent(m_pEmitters[i]->ToString(), 2);
		if (i + 1 < m_pEmitters.size())
		{
			EmitterString += ",";
		}
		EmitterString += "\n";
	}

	return tfm::format(
		"Scene[\n"
		"  acceleration = %s,\n"
		"  integrator = %s,\n"
		"  sampler = %s\n"
		"  camera = %s,\n"
		"  emitters = {\n"
		"  %s  },\n"
		"  meshes = {\n"
		"  %s  }\n"
		"]",
		Indent(m_pAcceleration->ToString()),
		Indent(m_pIntegrator->ToString()),
		Indent(m_pSampler->ToString()),
		Indent(m_pCamera->ToString()),
		Indent(EmitterString, 2),
		Indent(MeshesString, 2)
	);
}

Object::EClassType Scene::GetClassType() const
{
	return EClassType::EScene;
}

NAMESPACE_END
