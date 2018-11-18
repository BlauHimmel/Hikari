#include <core\Shape.hpp>

NAMESPACE_BEGIN

Mesh * Shape::GetMesh() const
{
	return nullptr;
}

uint32_t Shape::GetFacetIndex() const
{
	return uint32_t(-1);
}

Object::EClassType Shape::GetClassType() const
{
	return EClassType::EShape;
}

NAMESPACE_END