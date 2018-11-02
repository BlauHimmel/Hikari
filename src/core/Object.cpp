#include <core/Object.hpp>

NAMESPACE_BEGIN

Object::~Object() { /* Do nothing */ }

void Object::AddChild(Object * pChildObj)
{
	throw HikariException(
		"Object::AddChild(Object * pChildObj) is not implemented for objects of type '%s'!",
		ClassTypeName(GetClassType())
	);
}

void Object::SetParent(Object * pParentObj)
{
	throw HikariException(
		"Object::SetParent(Object * pParentObj) is not implemented for objects of type '%s'!",
		ClassTypeName(GetClassType())
	);
}

void Object::Activate() { /* Do nothing */ }

std::string Object::ClassTypeName(EClassType Type)
{
	switch (Type)
	{
		case Hikari::Object::EClassType::EScene:                return "Scene";
		case Hikari::Object::EClassType::EMesh:                 return "Mesh";
		case Hikari::Object::EClassType::EBSDF:                 return "BSDF";
		case Hikari::Object::EClassType::EPhaseFunction:        return "PhaseFunction";
		case Hikari::Object::EClassType::EEmitter:              return "Emitter";
		case Hikari::Object::EClassType::EMedium:               return "Medium";
		case Hikari::Object::EClassType::ECamera:               return "Camera";
		case Hikari::Object::EClassType::EIntegrator:           return "Integrator";
		case Hikari::Object::EClassType::ESampler:              return "Sampler";
		case Hikari::Object::EClassType::ETest:                 return "Test";
		case Hikari::Object::EClassType::EReconstructionFilter: return "ReconstructionFilter";
		default:                                                return "<Unknown>";
	}
}

std::map<std::string, ObjectFactory::Constructor> * ObjectFactory::m_pConstructors = nullptr;

void ObjectFactory::RegisterClz(const std::string & Name, const Constructor & Construct)
{
	if (m_pConstructors == nullptr)
	{
		m_pConstructors = new std::map<std::string, Constructor>();
	}
	(*m_pConstructors)[Name] = Construct;
}

Object * ObjectFactory::CreateInstance(const std::string & Name, const PropertyList & PropList)
{
	if (m_pConstructors == nullptr || m_pConstructors->find(Name) == m_pConstructors->end())
	{
		throw HikariException("A constructor for class \"%s\" could not be found!", Name);
	}
	return (*m_pConstructors)[Name](PropList);
}

NAMESPACE_END


