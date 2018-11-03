#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* \brief Superclass of all emitters
*/
class Emitter : public Object
{
public:
	/**
	* \brief Return the type of object (i.e. Mesh/Emitter/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const override;
};

NAMESPACE_END