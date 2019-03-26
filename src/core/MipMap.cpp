#include <core\MipMap.hpp>

NAMESPACE_BEGIN

float Lanczos(float X, float Tau)
{
	X = std::abs(X);
	float S = std::sin(X * Tau) / (X * Tau);
	float Lanczos = std::sin(X) / X;
	return S * Lanczos;
}

NAMESPACE_END
