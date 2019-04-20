#pragma once

#include <core\Common.hpp>

NAMESPACE_BEGIN

/**
 * \brief Utility class for evaluating the transmittance through rough
 * dielectric surfaces modeled using microfacet distributions.
 *
 * The transmittance through a rough dielectric boundary based on
 * a microfacet model depends on several quantities:
 *
 * 1. the relative index of refraction
 * 2. the angle of incidence (of incoming illumination)
 * 3. the roughness parameter of the microfacet distribution
 */
struct RoughTransmittance
{
	/**
	* \brief Load a rough transmittance data file from disk.
	* Data file were stored in: <root>\data\BeckmannRFTData.bin
	* and <root>\data\GGXRFTData.bin. While rendering, copy these
	* files to the same directory of the scene file.
	*/
	RoughTransmittance(const std::string & DataFilename);

	/**
	 * \brief Evaluate the rough transmittance for a given index of refraction,
	 * roughness, and angle of incidence.
	 *
	 * \param CosTheta
	 *     Cosine of the angle of incidence
	 * \param Alpha
	 *     Roughness parameter
	 * \param Eta
	 *     Relative index of refraction
	 */
	float Eval(float CosTheta, float Alpha = 0.0f, float Eta = 0.0f) const;

	/**
	 * \brief Evaluate the a diffuse rough transmittance for a given
	 * index of refraction, roughness, and angle of incidence.
	 *
	 * The diffuse rough transmittance is cosine-weighted integral
	 * of the rough transmittance over the incident hemisphere.
	 *
	 * \param Alpha
	 *     Roughness parameter
	 * \param Eta
	 *     Relative index of refraction
	 */
	float EvalDiffuse(float Alpha = 0.0f, float Eta = 0.0f) const;

	/**
	* \brief Reduce the internal 3D table to 2D by specializing
	* to a constant relative index of refraction
	*
	* Should only be called once!
	*/
	void SetEta(float Eta);

	/**
	 * \brief Reduce the internal 2D table (after a preceding call to
	 * SetEta) to 1D by specializing to a constant roughness
	 *
	 * Should only be called once!
	 */
	void SetAlpha(float Alpha);

	void CheckEta(float Eta) const;

	void CheckAlpha(float Alpha) const;

	std::unique_ptr<RoughTransmittance> Clone() const;

	uint64_t EtaSamples;
	uint64_t AlphaSamples;
	uint64_t ThetaSamples;
	bool bEtaFixed, bAlphaFixed;
	float EtaMin, EtaMax;
	float AlphaMin, AlphaMax;
	uint64_t TransmittanceSize;
	uint64_t DiffTransmittanceSize;
	std::vector<float> Transmittances;
	std::vector<float> DiffTransmittances;

private:
	RoughTransmittance() { }
};

NAMESPACE_END