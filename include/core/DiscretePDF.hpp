#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>

NAMESPACE_BEGIN

/**
* \brief Discrete probability distribution
*
* This data structure can be used to transform uniformly distributed
* samples to a stored discrete probability distribution.
*/
struct DiscretePDF1D
{
public:
	friend struct DiscretePDF2D;

	DiscretePDF1D(const float * pFunc, int Count);

	int Count() const;

	float SampleContinuous(float Sample, float * pPdf = nullptr, int * pIdx = nullptr) const;

	int SampleDiscrete(float Sample, float * pPdf = nullptr, float * pSampleRemapped = nullptr) const;

	float Pdf(int Index) const;

private:
	std::vector<float> m_Cdf, m_Func;
	float m_FuncIntegral;
};

/**
* \brief Discrete probability distribution
*
* This data structure can be used to transform uniformly distributed
* samples to a stored discrete probability distribution.
*/
struct DiscretePDF2D
{
public:
	/// Construct a 2D distribution by the array data
	DiscretePDF2D(float * pDatas, int Width, int Height);

	Point2f SampleContinuous(Point2f Sample, float * pPdf = nullptr, Point2i * Idx = nullptr) const;

	float Pdf(Point2f Point) const;

private:
	std::vector<std::unique_ptr<DiscretePDF1D>> m_pConditionalRow;
	std::unique_ptr<DiscretePDF1D> m_pMarginalCol;
};

NAMESPACE_END