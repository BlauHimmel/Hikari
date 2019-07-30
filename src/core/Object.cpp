#include <core/Object.hpp>

NAMESPACE_BEGIN

Object::~Object() { /* Do nothing */ }

void Object::AddChild(Object * pChildObj, const std::string & Name)
{
	throw HikariException(
		"Object::AddChild(Object * pChildObj, const std::string & Name)"
		" is not implemented for objects of type '%s'!",
		ClassTypeName(GetClassType())
	);
}

void Object::SetParent(Object * pParentObj, const std::string & Name)
{
	/* Don nothing */
}

void Object::Activate() { /* Do nothing */ }

std::string Object::ClassTypeName(EClassType Type)
{
	switch (Type)
	{
		case EClassType::EScene:                return "Scene";
		case EClassType::EMesh:                 return "Mesh";
		case EClassType::EBSDF:                 return "BSDF";
		case EClassType::EPhaseFunction:        return "PhaseFunction";
		case EClassType::EEmitter:              return "Emitter";
		case EClassType::ETexture:              return "Texture";
		case EClassType::EMedium:               return "Medium";
		case EClassType::ECamera:               return "Camera";
		case EClassType::EIntegrator:           return "Integrator";
		case EClassType::ESampler:              return "Sampler";
		case EClassType::ETest:                 return "Test";
		case EClassType::EReconstructionFilter: return "ReconstructionFilter";
		case EClassType::EAcceleration:         return "Acceleration";
		case EClassType::EShape:                return "Shape";
		default:                                return "<Unknown>";
	}
}

std::map<std::string, ObjectFactory::Constructor> * ObjectFactory::m_pConstructors = nullptr;
std::map<std::string, Object *> * ObjectFactory::m_pCreatedInstances = nullptr;

void ObjectFactory::RegisterClz(const std::string & Name, const Constructor & Construct)
{
	if (m_pConstructors == nullptr)
	{
		m_pConstructors = new std::map<std::string, Constructor>();
	}

	if (m_pConstructors->find(Name) != m_pConstructors->end())
	{
		LOG(ERROR) << tfm::format("This registration was abort because class '%s' has been registered before!", Name);
	}
	else
	{
		(*m_pConstructors)[Name] = Construct;
	}
}

Object * ObjectFactory::CreateInstance(const std::string & Name, const PropertyList & PropList)
{
	if (m_pConstructors == nullptr || m_pConstructors->find(Name) == m_pConstructors->end())
	{
		throw HikariException("A constructor for class \"%s\" could not be found!", Name);
	}

	if (m_pCreatedInstances == nullptr)
	{
		m_pCreatedInstances = new std::map<std::string, Object *>();
	}

	Object * pNewObj = (*m_pConstructors)[Name](PropList);

	time_t Time;
	time(&Time);
	char Buffer[64];
	strftime(Buffer, sizeof(Buffer), "%Y-%m-%d_%H:%M:%S", localtime(&Time));

	std::string NewObjName = "[" + std::to_string(m_pCreatedInstances->size()) + "]" + Name + Buffer;
	(*m_pCreatedInstances)[NewObjName] = pNewObj;

	return pNewObj;
}

void ObjectFactory::ReleaseAllocatedMemory()
{
	/* Currently, the lifttime of each object is managed by its parent object and 
	Scene is managed by the user manually. Uncomment code below and remove the 'delelte'
	statement in destructor of each object to let the Factory manage the lifetime 
	of all object. */

	//for (const auto & KV : *m_pCreatedInstances)
	//{
	//	Object * pObj = KV.second;
	//	delete pObj;
	//}

	delete m_pCreatedInstances;
	delete m_pConstructors;
}

NAMESPACE_END


