#include <core\ReconstructionFilter.hpp>

NAMESPACE_BEGIN

float ReconstructionFilter::GetRadius() const
{
	return m_Radius;
}

Object::EClassType ReconstructionFilter::GetClassType() const
{
	return EClassType::EReconstructionFilter;
}

NAMESPACE_END
