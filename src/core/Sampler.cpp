#include <core\Sampler.hpp>

NAMESPACE_BEGIN

size_t Sampler::GetSampleCount() const
{
	return m_SampleCount;
}

Object::EClassType Sampler::GetClassType() const
{
	return EClassType::ESampler;
}

NAMESPACE_END