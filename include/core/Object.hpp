#pragma once

#include <core\Common.hpp>
#include <core\PropertyList.hpp>

NAMESPACE_BEGIN

/**
* \brief Base class of all objects
*
* An object represents an instance that is part of
* a scene description, e.g. a scattering model or emitter.
*
* While an Object is constructing, functions will be called in 
* a sequence like:
* Constructor() -> AddChild() (multiple times) -> SetParent() -> Activate()
*/
class Object
{
public:
	enum EClassType
	{
		EScene                = 0,
		EMesh                 = 1,
		EBSDF                 = 2,
		EPhaseFunction        = 3,
		EEmitter              = 4,
		ETexture              = 5,
		EMedium               = 6,
		ECamera               = 7,
		EIntegrator           = 8,
		ESampler              = 9,
		ETest                 = 10,
		EReconstructionFilter = 11,
		EAcceleration         = 12,
		EShape                = 13,
		EClassTypeCount       = 14
	};

	/// Virtual destructor
	virtual ~Object();

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const = 0;

	/**
	* \brief Add a child object to the current instance
	*
	* The default implementation does not support children and
	* simply throws an exception
	*/
	virtual void AddChild(Object * pChildObj, const std::string & Name);

	/**
	* \brief Set the parent object
	*
	* Subclasses may choose to override this method to be
	* notified when they are added to a parent object. The
	* default implementation simply does nothing
	*/
	virtual void SetParent(Object * pParentObj, const std::string & Name);

	/**
	* \brief Perform some action associated with the object
	*
	* The default implementation does nothing. Certain objects
	* may choose to override it, e.g. to implement initialization,
	* testing, or rendering functionality.
	*
	* This function is called by the XML parser once it has
	* constructed an object and added all of its children
	* using \ref AddChild().
	*/
	virtual void Activate();

	/// Return a brief string summary of the instance (for debugging purposes)
	virtual std::string ToString() const = 0;

	/// Turn a class type into a human-readable string
	static std::string ClassTypeName(EClassType Type);
};

/**
* \brief Factory for objects
*
* This utility class is part of a mini-RTTI framework and can
* instantiate arbitrary Nori objects by their name.
*/
class ObjectFactory
{
public:
	using Constructor = std::function<Object * (const PropertyList &)>;

	/**
	* \brief Register an object constructor with the object factory
	*
	* This function is called by the macro \ref REGISTER_CLASS
	*
	* \param Name
	*     An internal name that is associated with this class. This is the
	*     'type' field found in the scene description XML files
	*
	* \param Construct
	*     A function pointer to an anonymous function that is
	*     able to call the constructor of the class.
	*/
	static void RegisterClz(const std::string & Name, const Constructor & Construct);

	/**
	* \brief Construct an instance from the class of the given name
	*
	* \param Name
	*     An internal name that is associated with this class. This is the
	*     'type' field found in the scene description XML files
	*
	* \param PropList
	*     A list of properties that will be passed to the constructor
	*     of the class.
	*/
	static Object * CreateInstance(const std::string & Name, const PropertyList & PropList);

	/**
	* \brief Release all the instance created by the function 'CreateInstance' and the 
	* memory used for the 'ObjectFactory' class.
	*/
	static void ReleaseAllocatedMemory();

private:
	static std::map<std::string, Constructor> * m_pConstructors;
	static std::map<std::string, Object *> * m_pCreatedInstances;
};

/// Macro for registering an object constructor with the \ref ObjectFactory
#define REGISTER_CLASS(Class, Name) \
	Class * __##Class##Constructor(const PropertyList & PropList) \
	{ \
		return new Class(PropList); \
	} \
	static struct __##Class##Register \
	{ \
		__##Class##Register() \
		{ \
			ObjectFactory::RegisterClz(Name, __##Class##Constructor); \
		} \
	} __##Class##Obj;\

NAMESPACE_END