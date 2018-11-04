#include <integrator\NormalIntegrator.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(NormalIntegrator, XML_INTEGRATOR_NORMAL);

NormalIntegrator::NormalIntegrator(const PropertyList & PropList)
{
	m_MyProperty = PropList.GetString("myProperty");
	LOG(INFO) << "Parameter value was : " << m_MyProperty << std::endl;
}

Color3f NormalIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & Ray) const
{
	return Color3f(0, 1, 0);
}

std::string NormalIntegrator::ToString() const
{
	return tfm::format(
		"NormalIntegrator[\n"
		"  myProperty = \"%s\"\n"
		"]",
		m_MyProperty
	);
}

NAMESPACE_END
