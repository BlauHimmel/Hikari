#include <core\DiscretePDF.hpp>

NAMESPACE_BEGIN

DiscretePDF1D::DiscretePDF1D(float * pDatas, int nEntries, bool bNormalize)
{
	Reserve(nEntries);
	Clear();

	for (int i = 0; i < nEntries; i++)
	{
		Append(pDatas[i]);
	}

	if (bNormalize)
	{
		Normalize();
	}
}

DiscretePDF1D::DiscretePDF1D(size_t nEntries)
{
	Reserve(nEntries);
	Clear();
}

void DiscretePDF1D::Clear()
{
	m_Cdf.clear();
	m_Cdf.push_back(0.0f);
	m_bNormalized = false;
}

void DiscretePDF1D::Reserve(size_t nEntries)
{
	m_Cdf.reserve(nEntries + 1);
}

void DiscretePDF1D::Append(float PdfValue)
{
	m_Cdf.push_back(m_Cdf[m_Cdf.size() - 1] + PdfValue);
}

size_t DiscretePDF1D::Size() const
{
	return m_Cdf.size() - 1;
}

float DiscretePDF1D::operator[](size_t iEntry) const
{
	return m_Cdf[iEntry + 1] - m_Cdf[iEntry];
}

bool DiscretePDF1D::IsNormalized() const
{
	return m_bNormalized;
}

float DiscretePDF1D::GetSum() const
{
	return m_Sum;
}

float DiscretePDF1D::GetNormalization() const
{
	return m_Normalization;
}

float DiscretePDF1D::Normalize()
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

size_t DiscretePDF1D::Sample(float SampleValue) const
{
	auto Iter = std::lower_bound(m_Cdf.begin(), m_Cdf.end(), SampleValue);
	size_t Index = size_t(std::max(ptrdiff_t(0), Iter - m_Cdf.begin() - 1));
	return std::min(Index, m_Cdf.size() - 2);
}

size_t DiscretePDF1D::Sample(float SampleValue, float & Pdf) const
{
	size_t Index = Sample(SampleValue);
	Pdf = operator[](Index);
	return Index;
}

size_t DiscretePDF1D::SampleReuse(float & SampleValue) const
{
	size_t Index = Sample(SampleValue);
	SampleValue = (SampleValue - m_Cdf[Index]) / (m_Cdf[Index + 1] - m_Cdf[Index]);
	return Index;
}

size_t DiscretePDF1D::SampleReuse(float & SampleValue, float & Pdf) const
{
	size_t Index = Sample(SampleValue, Pdf);
	SampleValue = (SampleValue - m_Cdf[Index]) / (m_Cdf[Index + 1] - m_Cdf[Index]);
	return Index;
}

std::string DiscretePDF1D::ToString() const
{
	std::string Result = tfm::format("DiscretePDF1D[sum = %f, normalized = %f, pdf = {", m_Sum, m_bNormalized);

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

DiscretePDF2D::DiscretePDF2D(float * pDatas, int Width, int Height)
{
	m_pConditionalRow.reserve(Height);
	for (int i = 0; i < Height; i++)
	{
		m_pConditionalRow.push_back(std::make_unique<DiscretePDF1D>(&pDatas[i * Width], Width));
	}

	std::vector<float> Marginal;
	Marginal.reserve(Height);
	for (int i = 0; i < Height; i++)
	{
		Marginal.push_back(m_pConditionalRow[i]->GetSum());
	}

	m_pMarginalCol.reset(new DiscretePDF1D(Marginal.data(), int(Marginal.size())));
}

float DiscretePDF2D::Pdf(Point2i Idx) const
{
	float PdfX = (*m_pMarginalCol)[Idx.y()];
	float PdfY = (*m_pConditionalRow[Idx.y()])[Idx.x()];
	return PdfX * PdfY;
}

Point2i DiscretePDF2D::Sample(Point2f Sample) const
{
	size_t IdxY = m_pMarginalCol->Sample(Sample.y());
	size_t IdxX = m_pConditionalRow[IdxY]->Sample(Sample.x());
	return Point2i(IdxX, IdxY);
}

Point2i DiscretePDF2D::Sample(Point2f Sample, float & Pdf) const
{
	float PdfX, PdfY;
	size_t IdxY = m_pMarginalCol->Sample(Sample.y(), PdfY);
	size_t IdxX = m_pConditionalRow[IdxY]->Sample(Sample.x(), PdfX);
	Pdf = PdfX * PdfY;
	return Point2i(IdxX, IdxY);
}

Point2i DiscretePDF2D::SampleReuse(Point2f & Sample) const
{
	size_t IdxY = m_pMarginalCol->SampleReuse(Sample.y());
	size_t IdxX = m_pConditionalRow[IdxY]->SampleReuse(Sample.x());
	return Point2i(IdxX, IdxY);
}

Point2i DiscretePDF2D::SampleReuse(Point2f & Sample, float & Pdf) const
{
	float PdfX, PdfY;
	size_t IdxY = m_pMarginalCol->SampleReuse(Sample.y(), PdfY);
	size_t IdxX = m_pConditionalRow[IdxY]->SampleReuse(Sample.x(), PdfX);
	Pdf = PdfX * PdfY;
	return Point2i(IdxX, IdxY);
}

NAMESPACE_END