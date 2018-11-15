#include <core\Shape.hpp>

NAMESPACE_BEGIN

Object::EClassType Shape::GetClassType() const
{
	return EClassType::EShape;
}

NAMESPACE_END