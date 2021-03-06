#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\Vector.hpp>

#if defined(__PLATFORM_LINUX__)
#include <malloc.h>
#endif

#if defined(__PLATFORM_WINDOWS__)
#include <windows.h>
#endif

#if defined(__PLATFORM_MACOS__)
#include <sys/sysctl.h>
#endif

NAMESPACE_BEGIN

int GetCoreCount()
{
	return int(std::thread::hardware_concurrency());
}

std::string Indent(const std::string & String, int Amount)
{
	std::istringstream ISS(String);
	std::ostringstream OSS;
	std::string Spacer(Amount, ' ');
	bool FirstLine = true;
	for (std::string Line; std::getline(ISS, Line); )
	{
		if (!FirstLine)
		{
			OSS << Spacer;
		}
		OSS << Line;
		if (!ISS.eof())
		{
			OSS << endl;
		}
		FirstLine = false;
	}
	return OSS.str();
}

std::string ToLower(const std::string & String)
{
	std::function<char(char)> ToLower = [&](char C)
	{
		return char(tolower(int(C)));
	};
	std::string Result;
	Result.resize(String.size());
	std::transform(String.begin(), String.end(), Result.begin(), ToLower);
	return Result;
}

bool ToBool(const std::string & String)
{
	std::string Value = ToLower(String);
	if (Value == "false")
	{
		return false;
	}
	else if (Value == "true")
	{
		return true;
	}
	else
	{
		throw HikariException("Could not parse boolean value \"%s\"", String);
	}
}

int ToInt(const std::string & String)
{
	char * pEnd = nullptr;
	int Result = int(strtol(String.c_str(), &pEnd, 10));
	if (*pEnd != '\0')
	{
		throw HikariException("Could not parse integer value \"%s\"", String);
	}
	return Result;
}

unsigned int ToUInt(const std::string & String)
{
	char * pEnd = nullptr;
	unsigned int Result = unsigned int(strtoul(String.c_str(), &pEnd, 10));
	if (*pEnd != '\0')
	{
		throw HikariException("Could not parse integer value \"%s\"", String);
	}
	return Result;
}

float ToFloat(const std::string & String)
{
	char * pEnd = nullptr;
	float Result = float(strtof(String.c_str(), &pEnd));
	if (*pEnd != '\0')
	{
		throw HikariException("Could not parse floating point value \"%s\"", String);
	}
	return Result;
}

Eigen::Vector3f ToVector3f(const std::string & String)
{
	std::vector<std::string> Tokens = Tokenize(String);
	if (Tokens.size() != 3)
	{
		throw HikariException("Expected 3 values");
	}
	Eigen::Vector3f Result;
	for (int i = 0; i < 3; ++i)
	{
		Result[i] = ToFloat(Tokens[i]);
	}
	return Result;
}

std::vector<std::string> Tokenize(const std::string & String, const std::string & Delim, bool bIncludeEmpty)
{
	std::string::size_type LastPos = 0, Pos = String.find_first_of(Delim, LastPos);
	std::vector<std::string> Tokens;

	while (LastPos != std::string::npos)
	{
		if (Pos != LastPos || bIncludeEmpty)
		{
			Tokens.push_back(String.substr(LastPos, Pos - LastPos));
		}
		LastPos = Pos;
		if (LastPos != std::string::npos)
		{
			LastPos += 1;
			Pos = String.find_first_of(Delim, LastPos);
		}
	}
	return Tokens;
}

bool EndsWith(const std::string & String, const std::string & Ending)
{
	if (Ending.size() > String.size())
	{
		return false;
	}
	return std::equal(Ending.rbegin(), Ending.rend(), String.rbegin());
}

std::string TimeString(double Time, bool bPrecise)
{
	if (std::isnan(Time) || std::isinf(Time))
	{
		return "inf";
	}

	std::string Suffix = "ms";
	if (Time > 1000)
	{
		Time /= 1000; Suffix = "s";
		if (Time > 60)
		{
			Time /= 60; Suffix = "m";
			if (Time > 60)
			{
				Time /= 60; Suffix = "h";
				if (Time > 24)
				{
					Time /= 24; Suffix = "d";
				}
			}
		}
	}

	std::ostringstream OS;
	OS << std::setprecision(bPrecise ? 4 : 1) << std::fixed << Time << Suffix;
	return OS.str();
}

std::string MemString(size_t Size, bool bPrecise)
{
	double Value = double(Size);
	const char * pSuffixes[] = 
	{
		"B", "KiB", "MiB", "GiB", "TiB", "PiB"
	};
	int Suffix = 0;
	while (Suffix < 5 && Value > 1024.0)
	{
		Value /= 1024.0; ++Suffix;
	}

	std::ostringstream OS;
	OS << std::setprecision(Suffix == 0 ? 0 : (bPrecise ? 4 : 1)) << std::fixed << Value << " " << pSuffixes[Suffix];
	return OS.str();
}

Color3f Clamp(Color3f Value, Color3f Min, Color3f Max)
{
	return Color3f(
		Clamp(Value[0], Min[0], Max[0]),
		Clamp(Value[1], Min[1], Max[1]),
		Clamp(Value[2], Min[2], Max[2])
	);
}

Color4f Clamp(Color4f Value, Color4f Min, Color4f Max)
{
	return Color4f(
		Clamp(Value[0], Min[0], Max[0]),
		Clamp(Value[1], Min[1], Max[1]),
		Clamp(Value[2], Min[2], Max[2]),
		Clamp(Value[3], Min[3], Max[3])
	);
}

Color3f Lerp(float T, const Color3f & V1, const Color3f & V2)
{
	return (1.0f - T) * V1 + T * V2;
}

Color4f Lerp(float T, const Color4f & V1, const Color4f & V2)
{
	return (1.0f - T) * V1 + T * V2;
}

float EvalCubicInterpolate1D(
	float X,
	const float * pValues,
	int Size,
	float Min,
	float Max,
	bool bExtrapolate
)
{
	/* Give up when given an out-of-range or NaN argument */
	if (!(X >= Min && X <= Max) && !bExtrapolate)
	{
		return 0.0f;
	}

	/* Transform X so that knots lie at integer positions */
	float T = ((X - Min) * (Size - 1)) / (Max - Min);

	/* Find the index of the left knot in the queried subinterval, be
	   robust to cases where T lies exactly on the right endpoint */
	int K = std::max(int(0), std::min(int(T), Size - 2));

	float F0 = pValues[K], F1 = pValues[K + 1];
	float D0, D1;

	/* Approximate the derivatives */
	if (K > 0)
	{
		D0 = 0.5f * (pValues[K + 1] - pValues[K - 1]);
	}
	else
	{
		D0 = pValues[K + 1] - pValues[K];
	}

	if (K + 2 < Size)
	{
		D1 = 0.5f * (pValues[K + 2] - pValues[K]);
	}
	else
	{
		D1 = pValues[K + 1] - pValues[K];
	}

	/* Compute the relative position within the interval */
	T = T - float(K);

	float T2 = T * T, T3 = T2 * T;

	return (2.0f * T3 - 3.0f * T2 + 1) * F0 +
		(-2.0f * T3 + 3.0f * T2) * F1 +
		(T3 - 2.0f * T2 + T) * D0 +
		(T3 - T2) * D1;
}

float EvalCubicInterpolate2D(
	const Point2f & P,
	const float * pValues,
	const Point2i & Size,
	const Point2f & Min,
	const Point2f & Max,
	bool bExtrapolate
)
{
	float KnotWeights[2][4];
	Point2i Knot;

	/* Compute interpolation weights separately for each dimension */
	for (int Dim = 0; Dim < 2; ++Dim)
	{
		float * pWeights = KnotWeights[Dim];
		/* Give up when given an out-of-range or NaN argument */
		if (!(P[Dim] >= Min[Dim] && P[Dim] <= Max[Dim]) && !bExtrapolate)
		{
			return 0.0f;
		}

		/* Transform P so that knots lie at integer positions */
		float T = ((P[Dim] - Min[Dim]) * float(Size[Dim] - 1)) / (Max[Dim] - Min[Dim]);

		/* Find the index of the left knot in the queried subinterval, be
		   robust to cases where 't' lies exactly on the right endpoint */
		Knot[Dim] = std::min(int(T), Size[Dim] - 2);

		/* Compute the relative position within the interval */
		T = T - float(Knot[Dim]);

		/* Compute node weights */
		float T2 = T * T, T3 = T2 * T;
		pWeights[0] = 0.0f;
		pWeights[1] = 2.0f * T3 - 3.0f * T2 + 1;
		pWeights[2] = -2.0f * T3 + 3.0f * T2;
		pWeights[3] = 0.0f;

		/* Derivative weights */
		float D0 = T3 - 2.0f * T2 + T, D1 = T3 - T2;

		/* Turn derivative weights into node weights using
		   an appropriate chosen finite differences stencil */
		if (Knot[Dim] > 0)
		{
			pWeights[2] += 0.5f * D0;
			pWeights[0] -= 0.5f * D0;
		}
		else
		{
			pWeights[2] += D0;
			pWeights[1] -= D0;
		}

		if (Knot[Dim] + 2 < Size[Dim])
		{
			pWeights[3] += 0.5f * D1;
			pWeights[1] -= 0.5f * D1;
		}
		else
		{
			pWeights[2] += D1;
			pWeights[1] -= D1;
		}
	}

	float Result = 0.0f;
	for (int y = -1; y <= 2; y++)
	{
		float WY = KnotWeights[1][y + 1];
		for (int x = -1; x <= 2; x++)
		{
			float WXY = KnotWeights[0][x + 1] * WY;

			if (WXY == 0)
			{
				continue;
			}

			int Pos = (Knot[1] + y) * Size[0] + Knot[0] + x;

			Result += pValues[Pos] * WXY;
		}
	}
	return Result;
}

float EvalCubicInterpolate3D(
	const Point3f & P,
	const float * pValues,
	const Point3i & Size,
	const Point3f & Min,
	const Point3f & Max,
	bool bExtrapolate
)
{
	float KnotWeights[3][4];
	Point3i Knot;

	/* Compute interpolation weights separately for each dimension */
	for (int Dim = 0; Dim < 3; ++Dim)
	{
		float * pWeights = KnotWeights[Dim];

		/* Give up when given an out-of-range or NaN argument */
		if (!(P[Dim] >= Min[Dim] && P[Dim] <= Max[Dim]) && !bExtrapolate)
		{
			return 0.0f;
		}

		/* Transform P so that knots lie at integer positions */
		float T = ((P[Dim] - Min[Dim]) * float(Size[Dim] - 1)) / (Max[Dim] - Min[Dim]);

		/* Find the index of the left knot in the queried subinterval, be
		   robust to cases where 'T' lies exactly on the right endpoint */
		Knot[Dim] = std::min(int(T), Size[Dim] - 2);

		/* Compute the relative position within the interval */
		T = T - float(Knot[Dim]);

		/* Compute node weights */
		float T2 = T * T, T3 = T2 * T;
		pWeights[0] = 0.0f;
		pWeights[1] = 2.0f * T3 - 3.0f * T2 + 1.0f;
		pWeights[2] = -2.0f * T3 + 3.0f * T2;
		pWeights[3] = 0.0f;

		/* Derivative weights */
		float D0 = T3 - 2.0f * T2 + T, D1 = T3 - T2;

		/* Turn derivative weights into node weights using
		   an appropriate chosen finite differences stencil */
		if (Knot[Dim] > 0)
		{
			pWeights[2] += 0.5f * D0;
			pWeights[0] -= 0.5f * D0;
		}
		else
		{
			pWeights[2] += D0;
			pWeights[1] -= D0;
		}

		if (Knot[Dim] + 2 < Size[Dim])
		{
			pWeights[3] += 0.5f * D1;
			pWeights[1] -= 0.5f * D1;
		}
		else
		{
			pWeights[2] += D1;
			pWeights[1] -= D1;
		}
	}

	float Result = 0.0f;
	for (int z = -1; z <= 2; z++)
	{
		float WZ = KnotWeights[2][z + 1];
		for (int y = -1; y <= 2; y++)
		{
			float WYZ = KnotWeights[1][y + 1] * WZ;
			for (int x = -1; x <= 2; x++)
			{
				float WXYZ = KnotWeights[0][x + 1] * WYZ;

				if (WXYZ == 0)
				{
					continue;
				}

				int Pos = ((Knot[2] + z) * Size[1] + (Knot[1] + y)) * Size[0] + Knot[0] + x;

				Result += pValues[Pos] * WXYZ;
			}
		}
	}

	return Result;
}

Vector3f SphericalDirection(float Theta, float Phi)
{
	float SinTheta, CosTheta, SinPhi, CosPhi;

	SinCos(Theta, &SinTheta, &CosTheta);
	SinCos(Phi, &SinPhi, &CosPhi);

	return Vector3f(
		SinTheta * CosPhi,
		SinTheta * SinPhi,
		CosTheta
	);
}

Point2f SphericalCoordinates(const Vector3f & Dir)
{
	Point2f Result(
		std::acos(Dir.z()),
		std::atan2(Dir.y(), Dir.x())
	);
	if (Result.y() < 0)
	{
		Result.y() += 2.0f * float(M_PI);
	}
	return Result;
}

float FresnelDielectric(float CosThetaI, float Eta, float InvEta, float & CosThetaT)
{
	if (Eta == 1.0f)
	{
		CosThetaT = -CosThetaI;
		return 0.0f;
	}

	/* Using Snell's law, calculate the squared sine of the
	angle between the normal and the transmitted ray */
	float Scale = (CosThetaI > 0.0f) ? InvEta : Eta;
	float CosThetaTSqr = 1.0f - (1.0f - CosThetaI * CosThetaI) * (Scale * Scale);

	/* Check for total internal reflection */
	if (CosThetaTSqr <= 0.0f)
	{
		CosThetaT = 0.0f;
		return 1.0f;
	}

	/* Find the absolute cosines of the incident/transmitted rays */
	float CosThetaII = std::abs(CosThetaI);
	float CosThetaTT = std::sqrt(CosThetaTSqr);

	float Rs = (CosThetaII - Eta * CosThetaTT)
		/ (CosThetaII + Eta * CosThetaTT);
	float Rp = (Eta * CosThetaII - CosThetaTT)
		/ (Eta * CosThetaII + CosThetaTT);

	CosThetaT = (CosThetaI > 0.0f) ? -CosThetaTT : CosThetaTT;

	/* No polarization -- return the unpolarized reflectance */
	return 0.5f * (Rs * Rs + Rp * Rp);
}

Color3f FresnelConductor(float CosThetaI, const Color3f & Eta, const Color3f & EtaK)
{
	float CosThetaI2 = CosThetaI * CosThetaI;
	float SinThetaI2 = 1.0f - CosThetaI2;
	Color3f Eta2 = Eta * Eta;
	Color3f EtaK2 = EtaK * EtaK;

	Color3f T0 = Eta2 - EtaK2 - Color3f(SinThetaI2);
	Color3f A2PlusB2 = (T0 * T0 + 4.0f * Eta2 * EtaK2).cwiseSqrt();
	Color3f T1 = A2PlusB2 + Color3f(CosThetaI2);
	Color3f A = (0.5f * (A2PlusB2 + T0)).cwiseSqrt();
	Color3f T2 = 2.0f * CosThetaI * A;
	Color3f Rs = (T1 - T2).cwiseQuotient(T1 + T2);

	Color3f T3 = CosThetaI2 * A2PlusB2 + Color3f(SinThetaI2 * SinThetaI2);
	Color3f T4 = T2 * SinThetaI2;
	Color3f Rp = Rs * (T3 - T4).cwiseQuotient(T3 + T4);

	return 0.5f * (Rp + Rs);
}

float ApproxFresnelDiffuseReflectance(float Eta)
{
	/**
	* An evalution of the accuracy led
	* to the following scheme, which cherry-picks
	* fits from two papers where they are best.	
	*/
	if (Eta < 1.0f)
	{
		/* Fit by Egan and Hilgeman (1973). Works
		reasonably well for "normal" IOR values (<2).
		Max rel. error in 1.0 - 1.5 : 0.1%
		Max rel. error in 1.5 - 2   : 0.6%
		Max rel. error in 2.0 - 5   : 9.5%
		*/
		return -1.4399f * (Eta * Eta) + 0.7099f * Eta + 0.6681f + 0.0636f / Eta;
	}
	else
	{
		/* Fit by d'Eon and Irving (2011)
		*
		* Maintains a good accuracy even for
		* unrealistic IOR values.
		*
		* Max rel. error in 1.0 - 2.0   : 0.1%
		* Max rel. error in 2.0 - 10.0  : 0.2%
		*/
		float InvEta = 1.0f / Eta,
			InvEta2 = InvEta * InvEta,
			InvEta3 = InvEta2 * InvEta,
			InvEta4 = InvEta3 * InvEta,
			InvEta5 = InvEta4 * InvEta;

		return 0.919317f - 3.4793f * InvEta
			+ 6.75335f * InvEta2
			- 7.80989f * InvEta3
			+ 4.98554f * InvEta4
			- 1.36881f * InvEta5;
	}
}

void CoordinateSystem(const Vector3f & Va, Vector3f & Vb, Vector3f & Vc)
{
	if (std::abs(Va.x()) > std::abs(Va.y()))
	{
		float InvLen = 1.0f / std::sqrt(Va.x() * Va.x() + Va.z() * Va.z());
		Vc = Vector3f(Va.z() * InvLen, 0.0f, -Va.x() * InvLen);
	}
	else
	{
		float InvLen = 1.0f / std::sqrt(Va.y() * Va.y() + Va.z() * Va.z());
		Vc = Vector3f(0.0f, Va.z() * InvLen, -Va.y() * InvLen);
	}
	Vb = Vc.cross(Va);
}

Vector3f Reflect(const Vector3f & Wi)
{
	return Vector3f(-Wi.x(), -Wi.y(), Wi.z());
}

Vector3f Refract(const Vector3f & Wi, float CosThetaT, float Eta, float InvEta)
{
	float Scale = -(CosThetaT < 0.0f ? InvEta : Eta);
	return Vector3f(Scale * Wi.x(), Scale * Wi.y(), CosThetaT);
}

Vector3f Reflect(const Vector3f & Wi, const Vector3f & M)
{
	return 2.0f * Wi.dot(M) * M - Wi;
}

Vector3f Refract(const Vector3f & Wi, const Vector3f & M, float CosThetaT, float Eta, float InvEta)
{
	Eta = (CosThetaT < 0.0f ? InvEta : Eta);
	return M * (Wi.dot(M) * Eta + CosThetaT) - Wi * Eta;
}

filesystem::resolver * GetFileResolver()
{
	static std::unique_ptr<filesystem::resolver> pResolver(new filesystem::resolver());
	return pResolver.get();
}

NAMESPACE_END
