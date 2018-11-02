#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\Vector.hpp>

#if defined(PLATFORM_LINUX)
#include <malloc.h>
#endif

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

#if defined(PLATFORM_MACOS)
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
				if (Time > 12)
				{
					Time /= 12; Suffix = "d";
				}
			}
		}
	}

	std::ostringstream OS;
	OS << std::setprecision(bPrecise ? 4 : 1) << std::fixed << time << Suffix;
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

float Fresnel(float CosThetaI, float ExtIOR, float IntIOR)
{
	float EtaI = ExtIOR, EtaT = IntIOR;

	if (ExtIOR == IntIOR)
	{
		return 0.0f;
	}

	/* Swap the indices of refraction if the interaction starts
	at the inside of the object */
	if (CosThetaI < 0.0f)
	{
		std::swap(EtaI, EtaT);
		CosThetaI = -CosThetaI;
	}

	/* Using Snell's law, calculate the squared sine of the
	angle between the normal and the transmitted ray */
	float eta = EtaI / EtaT;
	float SinThetaTSqr = eta * eta * (1.0f - CosThetaI * CosThetaI);

	if (SinThetaTSqr > 1.0f)
	{
		return 1.0f;  /* Total internal reflection! */
	}

	float CosThetaT = std::sqrt(1.0f - SinThetaTSqr);

	float Rs = (EtaI * CosThetaI - EtaT * CosThetaT)
		/ (EtaI * CosThetaI + EtaT * CosThetaT);
	float Rp = (EtaT * CosThetaI - EtaI * CosThetaT)
		/ (EtaT * CosThetaI + EtaI * CosThetaT);

	return (Rs * Rs + Rp * Rp) / 2.0f;
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

NAMESPACE_END
