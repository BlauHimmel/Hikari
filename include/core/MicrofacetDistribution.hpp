#pragma once

#include <core\Common.hpp>

NAMESPACE_BEGIN

class MicrofacetDistribution
{
public:
	enum EType
	{
		EBeckmann, EGGX
	};

	MicrofacetDistribution(EType Type, float Alpha);
	MicrofacetDistribution(EType Type, float AlphaU, float AlphaV);

	EType GetType() const { return m_Type; }
	float GetAlpha() const { return m_AlphaU; }
	float GetAlphaU() const { return m_AlphaU; }
	float GetAlphaV() const { return m_AlphaV; }
	bool IsAnisotropic() const { return m_AlphaU != m_AlphaV; }
	bool IsIsotropic() const { return m_AlphaU == m_AlphaV; }

	void ScaleAlpha(float Scale);
	float Eval(const Vector3f & M) const;
	Vector3f Sample(const Vector3f & Wi, const Point2f & Sample, float & Pdf) const;
	float Pdf(const Vector3f & M) const;
	float SmithG1(const Vector3f & V, const Vector3f & M) const;
	float G(const Vector3f & Wi, const Vector3f & Wo, const Vector3f & M) const;
	static std::string TypeName(EType Type);
	std::string ToString() const;

protected:
	// Pdf is with respect to the solid angles
	Vector3f SampleAll(const Point2f & Sample, float & Pdf) const;
	float PdfAll(const Vector3f & M) const;
	// Compute the effective roughness projected on direction V
	float ProjectRoughness(const Vector3f & V) const;

protected:
	EType m_Type;
	float m_AlphaU, m_AlphaV;
};

NAMESPACE_END