#pragma once

#include <core/Common.hpp>
#include <core/PropertyList.hpp>

NAMESPACE_BEGIN

class Object
{
public:
	enum class EClassType
	{
		EScene                = 0,
		EMesh                 = 1,
		EBSDF                 = 2,
		EPhaseFunction        = 3,
		EEmitter              = 4,
		EMedium               = 6,
		ECamera               = 7,
		EIntegrator           = 8,
		ESampler              = 9,
		ETest                 = 10,
		EReconstructionFilter = 11,
		EClassTypeCount       = 12
	};

	virtual ~Object();

	virtual EClassType GetClassType() const = 0;

	virtual void AddChild(Object * pChildObj);

	virtual void SetParent(Object * pParentObj);

	virtual void Activate();

	virtual std::string ToString() const = 0;

	static std::string ClassTypeName(EClassType Type);
};

class ObjectFactory
{
public:
	using Constructor = std::function<Object * (const PropertyList &)>;

	static void RegisterClass(const std::string & Name, const Constructor & Construct);

	static Object * CreateInstance(const std::string & Name, const PropertyList & PropList);

private:
	static std::map<std::string, Constructor> * m_pConstructors;
};

#define REGISTER_CLASS(Class, Name) \
	Class * __##Class##Constructor(const PropertyList & PropList) \
	{ \
		return new Class(PropList); \
	} \
	static struct __##Class##Register \
	{ \
		__##Class##Register() \
		{ \
			ObjectFactory::RegisterClass(Name, __##Class##Constructor); \
		} \
	} __##Class##Obj;\

NAMESPACE_END