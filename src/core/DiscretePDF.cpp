#include <core\DiscretePDF.hpp>

NAMESPACE_BEGIN

DiscretePDF::DiscretePDF(size_t nEntries)
{
	Reserve(nEntries);
	Clear();
}

void DiscretePDF::Clear()
{
	m_Cdf.clear();
	m_Cdf.push_back(0.0f);
	m_bNormalized = false;
}

void DiscretePDF::Reserve(size_t nEntries)
{
	m_Cdf.reserve(nEntries + 1);
}

void DiscretePDF::Append(float PdfValue)
{
	m_Cdf.push_back(m_Cdf[m_Cdf.size() - 1] + PdfValue);
}

size_t DiscretePDF::Size() const
{
	return m_Cdf.size() - 1;
}

float DiscretePDF::operator[](size_t iEntry) const
{
	return m_Cdf[iEntry + 1] - m_Cdf[iEntry];
}

bool DiscretePDF::IsNormalized() const
{
	return m_bNormalized;
}

float DiscretePDF::GetSum() const
{
	return m_Sum;
}

float DiscretePDF::GetNormalization() const
{
	return m_Normalization;
}

float DiscretePDF::Normalize()
{
	m_Sum = m_Cdf[m_Cdf.size() - 1];
	if (m_Sum > 0)
	{
		m_Normalization = 1.0f / m_Sum;
		for (size_t i = 1; i < m_Cdf.size(); ++i)
		{
			m_Cdf[i] *= m_Normalization;
		}
		m_Cdf[m_Cdf.size() - 1] = 1.0f;
		m_bNormalized = true;
	}
	else
	{
		m_Normalization = 0.0f;
	}
	return m_Sum;
}

size_t DiscretePDF::Sample(float SampleValue) const
{
	auto Iter = std::lower_bound(m_Cdf.begin(), m_Cdf.end(), SampleValue);
	size_t Index = size_t(std::max(ptrdiff_t(0), Iter - m_Cdf.begin() - 1));
	return std::min(Index, m_Cdf.size() - 2);
}

size_t DiscretePDF::Sample(float SampleValue, float & Pdf) const
{
	size_t Index = Sample(SampleValue);
	Pdf = operator[](Index);
	return Index;
}

size_t DiscretePDF::SampleReuse(float & SampleValue) const
{
	size_t Index = Sample(SampleValue);
	SampleValue = (SampleValue - m_Cdf[Index]) / (m_Cdf[Index + 1] - m_Cdf[Index]);
	return Index;
}

size_t DiscretePDF::SampleReuse(float & SampleValue, float & Pdf) const
{
	size_t Index = Sample(SampleValue, Pdf);
	SampleValue = (SampleValue - m_Cdf[Index]) / (m_Cdf[Index + 1] - m_Cdf[Index]);
	return Index;
}

std::string DiscretePDF::ToString() const
{
	std::string Result = tfm::format("DiscretePDF[sum = %f, normalized = %f, pdf = {", m_Sum, m_bNormalized);

	for (size_t i = 0; i < m_Cdf.size(); ++i)
	{
		Result += std::to_string(operator[](i));
		if (i != m_Cdf.size() - 1)
		{
			Result += ", ";
		}
	}
	return Result + "}]";
}

NAMESPACE_END
