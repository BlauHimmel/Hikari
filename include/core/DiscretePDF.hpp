#pragma once

#include <core\Common.hpp>

NAMESPACE_BEGIN

/**
* \brief Discrete probability distribution
*
* This data structure can be used to transform uniformly distributed
* samples to a stored discrete probability distribution.
*/
struct DiscretePDF
{
public:
	/// Allocate memory for a distribution with the given number of entries
	explicit DiscretePDF(size_t nEntries = 0);

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

NAMESPACE_END