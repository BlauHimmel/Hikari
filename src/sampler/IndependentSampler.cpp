#include <sampler\IndependentSampler.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(IndependentSampler, "Independent");

IndependentSampler::IndependentSampler(const PropertyList & PropList)
{
	m_SampleCount = (size_t)PropList.GetInteger("SampleCount", 1);
}

std::unique_ptr<Sampler> IndependentSampler::Clone() const
{
	std::unique_ptr<IndependentSampler> Cloned(new IndependentSampler());
	Cloned->m_SampleCount = m_SampleCount;
	Cloned->m_Random = m_Random;
	return Cloned;
}

void IndependentSampler::Prepare(const ImageBlock & Block)
{
	m_Random.seed(Block.GetOffset().x(), Block.GetOffset().y());
}

void IndependentSampler::Generate()
{
	/* No-op for this sampler */
}

void IndependentSampler::Advance()
{
	/* No-op for this sampler */
}

float IndependentSampler::Next1D()
{
	return m_Random.nextFloat();
}

Point2f IndependentSampler::Next2D()
{
	return Point2f(m_Random.nextFloat(), m_Random.nextFloat());
}

std::string IndependentSampler::ToString() const
{
	return tfm::format("IndependentSampler[SampleCount = %i]", m_SampleCount);
}

IndependentSampler::IndependentSampler()
{

}

NAMESPACE_END

