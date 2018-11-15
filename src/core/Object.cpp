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


