#include <core\Integrator.hpp>

NAMESPACE_BEGIN

void Integrator::Preprocess(const Scene * pScene)
{

}

Object::EClassType Integrator::GetClassType() const
{
	return EClassType::EIntegrator;
}

NAMESPACE_END
