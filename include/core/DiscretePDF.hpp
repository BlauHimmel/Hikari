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

	/// Construct a 1D distribution by the array data
	DiscretePDF1D(float * pDatas, int nEntries, bool bNormalize = true);

	/// Allocate memory for a distribution with the given number of entries
	explicit DiscretePDF1D(size_t nEntries = 0);

	/// Clear all entries
	void Clear();

	/// Reserve memory for a certain number of entries
	void Reserve(size_t nEntries);

	/// Append an entry with the specified discrete probability
	void Append(float PdfValue);

	/// Return the number of entries so far
	size_t Size() const;

	/// Access an entry by its index
	float operator[](size_t iEntry) const;

	/// Have the probability densities been normalized?
	bool IsNormalized() const;

	/**
	* \brief Return the original (unnormalized) sum of all PDF entries
	*
	* This assumes that \ref Normalize() has previously been called
	*/
	float GetSum() const;

	/**
	* \brief Return the normalization factor (i.e. the inverse of \ref GetSum())
	*
	* This assumes that \ref Normalize() has previously been called
	*/
	float GetNormalization() const;

	/**
	* \brief Normalize the distribution
	*
	* \return Sum of the (previously unnormalized) entries
	*/
	float Normalize();

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* \param[in] SampleValue
	*     An uniformly distributed sample on [0,1]
	* \return
	*     The discrete index associated with the sample
	*/
	size_t Sample(float SampleValue) const;

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* \param[in] SampleValue
	*     An uniformly distributed sample on [0,1]
	* \param[out] Pdf
	*     Probability value of the sample
	* \return
	*     The discrete index associated with the sample
	*/
	size_t Sample(float SampleValue, float & Pdf) const;

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* The original sample is value adjusted so that it can be "reused".
	*
	* \param[in, out] SampleValue
	*     An uniformly distributed sample on [0,1]
	* \return
	*     The discrete index associated with the sample
	*/
	size_t SampleReuse(float & SampleValue) const;

	/**
	* \brief %Transform a uniformly distributed sample.
	*
	* The original sample is value adjusted so that it can be "reused".
	*
	* \param[in,out] SampleValue
	*     An uniformly distributed sample on [0,1]
	* \param[out] Pdf
	*     Probability value of the sample
	* \return
	*     The discrete index associated with the sample
	*/
	size_t SampleReuse(float & SampleValue, float & Pdf) const;

	/**
	* \brief Turn the underlying distribution into a
	* human-readable string format
	*/
	std::string ToString() const;

private:
	std::vector<float> m_Cdf;
	float m_Sum, m_Normalization;
	bool m_bNormalized;
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

	/// Access an entry by its index
	float Pdf(Point2i Sample) const;

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* \param[in] Sample
	*     An uniformly distributed sample on [0,1]^2
	* \return
	*     The discrete index associated with the sample
	*/
	Point2i Sample(Point2f Sample) const;

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* \param[in] Sample
	*     An uniformly distributed sample on [0,1]^2
	* \param[out] Pdf
	*     Probability value of the sample
	* \return
	*     The discrete index associated with the sample
	*/
	Point2i Sample(Point2f Sample, float & Pdf) const;

	/**
	* \brief %Transform a uniformly distributed sample to the stored distribution
	*
	* The original sample is value adjusted so that it can be "reused".
	*
	* \param[in, out] Sample
	*     An uniformly distributed sample on [0,1]^2
	* \return
	*     The discrete index associated with the sample
	*/
	Point2i SampleReuse(Point2f & Sample) const;

	/**
	* \brief %Transform a uniformly distributed sample.
	*
	* The original sample is value adjusted so that it can be "reused".
	*
	* \param[in,out] Sample
	*     An uniformly distributed sample on [0,1]^2
	* \param[out] Pdf
	*     Probability value of the sample
	* \return
	*     The discrete index associated with the sample
	*/
	Point2i SampleReuse(Point2f & Sample, float & Pdf) const;

private:
	std::vector<std::unique_ptr<DiscretePDF1D>> m_pConditionalRow;
	std::unique_ptr<DiscretePDF1D> m_pMarginalCol;
};

NAMESPACE_END