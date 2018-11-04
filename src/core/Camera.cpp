#include <core\Camera.hpp>

NAMESPACE_BEGIN

const Vector2i & Camera::GetOutputSize() const
{
	return m_OutputSize;
}

const ReconstructionFilter * Camera::GetReconstructionFilter() const
{
	return m_pFilter;
}

Object::EClassType Camera::GetClassType() const
{
	return EClassType::ECamera;
}

NAMESPACE_END


