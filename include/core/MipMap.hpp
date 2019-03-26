#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>
#include <core\MemoryArena.hpp>
#include <tbb\tbb.h>
#include <thread>

NAMESPACE_BEGIN

enum class EWrapMode
{
	ERepeat, EBlack, EClamp
};

struct ResampleWeight
{
	int FirstTexel;
	float Weight[4];
};

float Lanczos(float X, float Tau);

template <typename T>
class MipMap
{
public:
	MipMap(
		const Point2i & Resolution,
		const T * pData,
		bool bTrilinear = false,
		float MaxAnisotropic = 8.0f,
		EWrapMode WrapMode = EWrapMode::ERepeat
	);

	int GetWidth() const { return m_Resolution[0]; }
	int GetHeight() const { return m_Resolution[1]; }
	int GetLevels() const { return m_Pyramid.size(); }
	const T & Texel(int Level, int U, int V) const;
	T Lookup(const Point2f & UV, float Width = 0.0f) const;
	T Lookup(const Point2f & UV, Vector2f D0, Vector2f D1);

private:

	std::unique_ptr<ResampleWeight[]> GetResampleWeights(int Old, int New);

	T Triangle(int Level, const Point2f & UV) const;
	T EWA(int Level, Point2f UV, Vector2f D0, Vector2f D1) const;

	bool m_bTrilinear;
	float m_MaxAnisotropic;
	EWrapMode m_WrapMode;
	Point2i m_Resolution;
	std::vector<std::unique_ptr<BlockedArray<T, 2>>> m_Pyramid;

	/* EWA filter weights */
	static const int s_WeightLUTSize = 128;
	static float s_WeightLut[s_WeightLUTSize];
};

template<typename T>
inline MipMap<T>::MipMap(
	const Point2i & Resolution,
	const T * pData,
	bool bTrilinear,
	float MaxAnisotropic,
	EWrapMode WrapMode
) :
	m_bTrilinear(bTrilinear),
	m_MaxAnisotropic(MaxAnisotropic),
	m_WrapMode(WrapMode),
	m_Resolution(Resolution)
{
	static_assert(
		std::is_same<T, float>::value ||
		std::is_same<T, Color3f>::value ||
		std::is_same<T, Color4f>::value,
		"Type T of MipMap must be float, Color3f or Color4f!"
	);

	std::unique_ptr<T[]> ResampledImage = nullptr;

	if (!IsPowerOf2(m_Resolution[0]) || !IsPowerOf2(m_Resolution[1]))
	{
		Point2i ResolutionPow2(RoundUpPow2(m_Resolution[0]), RoundUpPow2(m_Resolution[1]));
		LOG(INFO) << "Resampling MipMap from " << m_Resolution.ToString() << " to " << ResolutionPow2.ToString() <<
			". Ratio = " << float(ResolutionPow2.x() * ResolutionPow2.y()) / float(m_Resolution.x() * m_Resolution.y());

		// Resamp image in U direction
		std::unique_ptr<ResampleWeight[]> UWeigths = GetResampleWeights(m_Resolution[0], ResolutionPow2[0]);
		ResampledImage.reset(new T[ResolutionPow2[0] * ResolutionPow2[1]]);

		tbb::blocked_range<int> UResampleRange(0, m_Resolution[1]);
		auto UResampleMap = [&](const tbb::blocked_range<int> & Range)
		{
			for (int v = Range.begin(); v < Range.end(); v++)
			{
				for (int u = 0; u < ResolutionPow2[0]; u++)
				{
					ResampledImage[v * ResolutionPow2[0] + u] = T(0.0f);
					for (int j = 0; j < 4; j++)
					{
						int OriginalU = UWeigths[u].FirstTexel + j;
						
						if (m_WrapMode == EWrapMode::ERepeat)
						{
							OriginalU = Mod(OriginalU, m_Resolution[0]);
						}
						else if (m_WrapMode == EWrapMode::EClamp)
						{
							OriginalU = Clamp(OriginalU, 0, m_Resolution[0] - 1);
						}

						if (OriginalU >= 0 && OriginalU < m_Resolution[0])
						{
							ResampledImage[v * ResolutionPow2[0] + u] +=
								UWeigths[u].Weight[j] *
								pData[v * m_Resolution[0] + OriginalU];
						}
					}
				}
			}
		};
		tbb::parallel_for(UResampleRange, UResampleMap);

		// Resamp image in V direction
		std::unique_ptr<ResampleWeight[]> VWeigths = GetResampleWeights(m_Resolution[1], ResolutionPow2[1]);
		std::vector<T *> ResampleBuffer;
		int nThread = int(std::thread::hardware_concurrency());
		for (int i = 0; i < nThread; i++)
		{
			ResampleBuffer.push_back(new T[ResolutionPow2[1]]);
		}

		tbb::blocked_range<int> VResampleRange(0, ResolutionPow2[0]);
		auto VResampleMap = [&](const tbb::blocked_range<int> & Range)
		{
			for (int u = Range.begin(); u < Range.end(); u++)
			{
				T * pWorkData = ResampleBuffer[tbb::task_arena::current_thread_index()];
				for (int v = 0; v < ResolutionPow2[1]; v++)
				{
					pWorkData[v] = T(0.0f);
					for (int j = 0; j < 4; j++)
					{
						int Offset = VWeigths[v].FirstTexel + j;
						if (m_WrapMode == EWrapMode::ERepeat)
						{
							Offset = Mod(Offset, m_Resolution[1]);
						}
						else if (m_WrapMode == EWrapMode::EClamp)
						{
							Offset = Clamp(Offset, 0, m_Resolution[1] - 1);
						}

						if (Offset >= 0 && Offset < m_Resolution[1])
						{
							pWorkData[v] += VWeigths[v].Weight[j] *
								ResampledImage[Offset * ResolutionPow2[0] + u];
						}
					}
				}
				for (int v = 0; v < ResolutionPow2[1]; v++)
				{
					ResampledImage[v * ResolutionPow2[0] + u] = Clamp(pWorkData[v], T(0.0f), T(std::numeric_limits<float>::max()));
				}
			}
		};
		tbb::parallel_for(VResampleRange, VResampleMap);
		for (auto Ptr : ResampleBuffer)
		{
			delete[] Ptr;
		}
		m_Resolution = ResolutionPow2;
	}

	int nLevels = 1 + Log2Int(std::max(m_Resolution[0], m_Resolution[1]));
	m_Pyramid.resize(nLevels);

	// Initialize most detailed level of MIPMap
	m_Pyramid[0].reset(new BlockedArray<T, 2>(
		m_Resolution[0],
		m_Resolution[1],
		ResampledImage != nullptr ? ResampledImage.get() : pData)
	);

	for (int i = 1; i < nLevels; ++i)
	{
		int URes = std::max(1, m_Pyramid[i - 1]->USize() / 2);
		int VRes = std::max(1, m_Pyramid[i - 1]->VSize() / 2);
		m_Pyramid[i].reset(new BlockedArray<T, 2>(URes, VRes));

		tbb::blocked_range<int> Range(0, VRes);
		auto Map = [&](const tbb::blocked_range<int> & Range)
		{
			for (int v = Range.begin(); v < Range.end(); v++)
			{
				for (int u = 0; u < URes; u++)
				{
					(*m_Pyramid[i])(u, v) = 0.25f * (
						Texel(i - 1, 2 * u, 2 * v) +
						Texel(i - 1, 2 * u + 1, 2 * v) +
						Texel(i - 1, 2 * u, 2 * v + 1) +
						Texel(i - 1, 2 * u + 1, 2 * v + 1)
					);
				}
			}
		};
		tbb::parallel_for(Range, Map);
	}

	// Initialize EWA filter weights
	if (s_WeightLut[0] == 0.0f)
	{
		for (int i = 0; i < s_WeightLUTSize; i++)
		{
			float Alpha = 2.0f;
			float R2 = float(i) / float(s_WeightLUTSize - 1);
			s_WeightLut[i] = std::exp(-Alpha * R2) - std::exp(-Alpha);
		}
	}
}

template<typename T>
inline const T & MipMap<T>::Texel(int Level, int U, int V) const
{
	CHECK_LT(Level, m_Pyramid.size());

	const BlockedArray<T, 2> & Data = *m_Pyramid[Level];

	switch (m_WrapMode)
	{
	case EWrapMode::ERepeat:
		U = Mod(U, Data.USize());
		V = Mod(V, Data.VSize());
		break;
	case EWrapMode::EClamp:
		U = Clamp(U, 0, Data.USize() - 1);
		V = Clamp(V, 0, Data.VSize() - 1);
		break;
	case EWrapMode::EBlack:
		if (U < 0 || U >= int(Data.USize()) ||
			V < 0 || V >= int(Data.VSize()))
		{
			static const T Black(0.0f);
			return Black;
		}
		break;
	}

	return Data(U, V);
}

template<typename T>
inline T MipMap<T>::Lookup(const Point2f & UV, float Width) const
{
	// Compute MipMap level for trilinear filtering
	float Level = GetLevels() - 1.0f + std::log2(std::max(Width, 1e-8f));

	// Perform trilinear interpolation at appropriate MipMap level
	if (Level < 0.0f)
	{
		return Triangle(0, UV);
	}
	else if (Level >= GetLevels() - 1)
	{
		return Texel(GetLevels() - 1, 0, 0);
	}
	else
	{
		int iLevel = std::floor(Level);
		float Delta = Level - iLevel;
		return Lerp(Delta, Triangle(iLevel, UV), Triangle(iLevel + 1, UV));
	}
}

template<typename T>
inline T MipMap<T>::Lookup(const Point2f & UV, Vector2f D0, Vector2f D1)
{
	if (m_bTrilinear)
	{
		float Width = std::max(
			std::max(std::abs(D0[0]), std::abs(D0[1])),
			std::max(std::abs(D1[0]), std::abs(D1[1]))
		);

		return Lookup(UV, 2.0f * Width);
	}

	// Compute ellipse minor and major axes
	if (D0.squaredNorm() < D1.squaredNorm())
	{
		std::swap(D0, D1);
	}

	float MajorLength = D0.norm();
	float MinorLength = D1.norm();

	// Clamp ellipse eccentricity if too large
	if (MinorLength * m_MaxAnisotropic < MajorLength && MinorLength > 0.0f)
	{
		float Scale = MajorLength / (MinorLength * m_MaxAnisotropic);
		D1 *= Scale;
		MinorLength *= Scale;
	}

	if (MinorLength == 0.0f) 
	{
		return Triangle(0, UV);
	}

	// Choose level of detail for EWA lookup and perform EWA filtering
	float LOD = std::max(0.0f, GetLevels() - 1.0f + std::log2(MinorLength));
	int iLOD = std::floor(LOD);

	return Lerp(LOD - iLOD, EWA(iLOD, UV, D0, D1),
		EWA(iLOD + 1, UV, D0, D1));
}

template<typename T>
inline std::unique_ptr<ResampleWeight[]> MipMap<T>::GetResampleWeights(int Old, int New)
{
	CHECK_GE(New, Old);

	std::unique_ptr<ResampleWeight[]> Weights(new ResampleWeight[New]);
	float FilterWidth = 2.0f;

	for (int i = 0; i < New; i++)
	{
		// Compute image resampling weights for i th texel
		float Center = (i + 0.5f) * Old / New;
		Weights[i].FirstTexel = int(std::floor((Center - FilterWidth) + 0.5f));

		for (int j = 0; j < 4; j++)
		{
			float Position = Weights[i].FirstTexel + j + 0.5f;
			Weights[i].Weight[j] = Lanczos((Position - Center) / FilterWidth, 2.0f);
		}

		// Normalize filter weights for texel resampling
		float InvSumWeight = 1.0f / (Weights[i].Weight[0] + Weights[i].Weight[1] + Weights[i].Weight[2] + Weights[i].Weight[3]);
		for (int j = 0; j < 4; j++)
		{
			Weights[i].Weight[j] *= InvSumWeight;
		}
	}

	return Weights;
}

template<typename T>
inline T MipMap<T>::Triangle(int Level, const Point2f & UV) const
{
	Level = Clamp(Level, 0, GetLevels() - 1);
	float U = UV[0] * m_Pyramid[Level]->USize() - 0.5f;
	float V = UV[1] * m_Pyramid[Level]->VSize() - 0.5f;
	int U0 = std::floor(U);
	int V0 = std::floor(V);
	float dU = U - U0;
	float dV = V - V0;
	return (1.0f - dU) * (1.0f - dV) * Texel(Level, U0, V0) +
		(1.0f - dU) * dV * Texel(Level, U0, V0 + 1) +
		dU * (1.0f - dV) * Texel(Level, U0 + 1, V0) +
		dU * dV * Texel(Level, U0 + 1, V0 + 1);
}

template<typename T>
inline T MipMap<T>::EWA(int Level, Point2f UV, Vector2f D0, Vector2f D1) const
{
	if (Level >= GetLevels())
	{
		return Texel(GetLevels() - 1, 0, 0);
	}

	UV[0] = UV[0] * m_Pyramid[Level]->USize() - 0.5f;
	UV[1] = UV[1] * m_Pyramid[Level]->VSize() - 0.5f;

	D0[0] *= m_Pyramid[Level]->USize();
	D0[1] *= m_Pyramid[Level]->VSize();
	D1[0] *= m_Pyramid[Level]->USize();
	D1[1] *= m_Pyramid[Level]->VSize();

	// Compute ellipse coefficients to bound EWA filter region
	float A = D0[1] * D0[1] + D1[1] * D1[1] + 1.0f;
	float B = -2.0f * (D0[0] * D0[1] + D1[0] * D1[1]);
	float C = D0[0] * D0[0] + D1[0] * D1[0] + 1.0f;
	float InvF = 1.0f / (A * C - B * B * 0.25f);
	A *= InvF;
	B *= InvF;
	C *= InvF;

	// Compute the ellipse's (u,v) bounding box in texture space
	float Det = -B * B + 4.0f * A * C;
	float InvDet = 1.0f / Det;
	float USqrt = std::sqrt(Det * C);
	float VSqrt = std::sqrt(A * Det);
	int U0 = std::ceil(UV[0] - 2.0f * InvDet * USqrt);
	int U1 = std::floor(UV[0] + 2.0f * InvDet * USqrt);
	int V0 = std::ceil(UV[1] - 2.0f * InvDet * VSqrt);
	int V1 = std::floor(UV[1] + 2.0f * InvDet * VSqrt);

	// Scan over ellipse bound and compute quadratic equation
	T Sum(0.0f);
	float SumWeight = 0.0f;
	for (int iV = V0; iV <= V1; ++iV)
	{
		float dV = iV - UV[1];
		for (int iU = U0; iU <= U1; ++iU)
		{
			float dU = iU - UV[0];
			// Compute squared radius and filter texel if inside ellipse
			float R2 = A * dU * dU + B * dU * dV + C * dV * dV;
			if (R2 < 1.0f)
			{
				int Index = std::min((int)(R2 * s_WeightLUTSize), s_WeightLUTSize - 1);
				float Weight = s_WeightLut[Index];
				Sum += Texel(Level, iU, iV) * Weight;
				SumWeight += Weight;
			}
		}
	}

	return Sum / SumWeight;
}

template<typename T>
float MipMap<T>::s_WeightLut[s_WeightLUTSize] = { 0.0f };

NAMESPACE_END


