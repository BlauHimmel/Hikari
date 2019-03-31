#include <core\DiscretePDF.hpp>

NAMESPACE_BEGIN

DiscretePDF1D::DiscretePDF1D(const float * pFunc, int Count) :
	m_Func(pFunc, pFunc + Count), m_Cdf(Count + 1)
{
	m_Cdf[0] = 0.0f;
	for (int i = 1; i < Count + 1; i++)
	{
		m_Cdf[i] = m_Cdf[i - 1] + m_Func[i - 1] / Count;
	}

	m_FuncIntegral = m_Cdf[Count];

	if (m_FuncIntegral == 0.0f)
	{
		for (int i = 1; i < Count + 1; i++)
		{
			m_Cdf[i] = float(i) / float(Count);
		}
	}
	else
	{
		for (int i = 1; i < Count + 1; i++)
		{
			m_Cdf[i] /= m_FuncIntegral;
		}
	}
}

int DiscretePDF1D::Count() const
{
	return int(m_Func.size());
}

float DiscretePDF1D::SampleContinuous(float Sample, float * pPdf, int * pIdx) const
{
	auto Iter = std::lower_bound(m_Cdf.begin(), m_Cdf.end(), Sample);
	int Idx = int(std::min(
		size_t(std::max(std::ptrdiff_t(0), Iter - m_Cdf.begin() - 1)), 
		m_Cdf.size() - 2
	));

	if (pIdx != nullptr)
	{
		*pIdx = Idx;
	}

	float Du = Sample - m_Cdf[Idx];
	if (m_Cdf[Idx + 1] - m_Cdf[Idx] > 0.0f)
	{
		Du /= (m_Cdf[Idx + 1] - m_Cdf[Idx]);
	}
	CHECK(!std::isnan(Du));

	if (pPdf != nullptr)
	{
		*pPdf = m_Func[Idx] / m_FuncIntegral;
	}

	return (float(Idx) + Du) / float(Count());
}

int DiscretePDF1D::SampleDiscrete(float Sample, float * pPdf, float * pSampleRemapped) const
{
	auto Iter = std::lower_bound(m_Cdf.begin(), m_Cdf.end(), Sample);
	int Idx = int(std::min(
		size_t(std::max(std::ptrdiff_t(0), Iter - m_Cdf.begin() - 1)),
		m_Cdf.size() - 2
	));

	if (pPdf != nullptr)
	{
		*pPdf = m_Func[Idx] / (m_FuncIntegral * float(Count()));
	}

	if (pSampleRemapped != nullptr)
	{
		*pSampleRemapped = (Sample - m_Cdf[Idx]) / (m_Cdf[Idx + 1] - m_Cdf[Idx]);
		CHECK(*pSampleRemapped <= 1.0f && *pSampleRemapped >= 0.0f);
	}

	return Idx;
}

float DiscretePDF1D::Pdf(int Index) const
{
	CHECK(Index >= 0 && Index < Count());
	return m_Func[Index] / (m_FuncIntegral * float(Count()));
}

DiscretePDF2D::DiscretePDF2D(float * pDatas, int Width, int Height)
{
	m_pConditionalRow.reserve(Height);
	for (int i = 0; i < Height; i++)
	{
		m_pConditionalRow.emplace_back(new DiscretePDF1D(&pDatas[i * Width], Width));
	}

	std::vector<float> Marginal;
	Marginal.reserve(Height);
	for (int i = 0; i < Height; i++)
	{
		Marginal.push_back(m_pConditionalRow[i]->m_FuncIntegral);
	}
	m_pMarginalCol.reset(new DiscretePDF1D(Marginal.data(), Height));
}

Point2f DiscretePDF2D::SampleContinuous(Point2f Sample, float * pPdf, Point2i * Idx) const
{
	float PdfX, PdfY;
	int IdxX, IdxY;
	float Dy = m_pMarginalCol->SampleContinuous(Sample.y(), &PdfY, &IdxY);
	float Dx = m_pConditionalRow[IdxY]->SampleContinuous(Sample.x(), &PdfX, &IdxX);
	if (pPdf != nullptr)
	{
		*pPdf = PdfX * PdfY;
	}
	if (Idx != nullptr)
	{
		*Idx = Point2i(IdxX, IdxY);
	}
	return Point2f(Dx, Dy);
}

float DiscretePDF2D::Pdf(Point2f Point) const
{
	int X = Clamp(int(Point.x() * m_pConditionalRow[0]->Count()), 0, m_pConditionalRow[0]->Count() - 1);
	int Y = Clamp(int(Point.y() * m_pMarginalCol->Count()), 0, m_pMarginalCol->Count() - 1);
	return m_pConditionalRow[Y]->m_Func[X] / m_pMarginalCol->m_FuncIntegral;
}

NAMESPACE_END