#include <core\Frame.hpp>

NAMESPACE_BEGIN

Frame::Frame() { }

Frame::Frame(const Normal3f & N, const Vector3f & Dpdu)
{
	this->N = N;
	this->S = (Dpdu - this->N * this->N.dot(Dpdu)).normalized();
	this->T = this->N.cross(this->S);
}

Frame::Frame(const Vector3f & S, const Vector3f & T, const Normal3f & N) : S(S), T(T), N(N) { }

Frame::Frame(const Vector3f & X, const Vector3f & Y, const Vector3f & Z) : S(X), T(Y), N(Z) { }

Frame::Frame(const Vector3f & N) : N(N)
{
	CoordinateSystem(N, S, T);
}

Vector3f Frame::ToLocal(const Vector3f & V) const
{
	return Vector3f(V.dot(S), V.dot(T), V.dot(N));
}

Vector3f Frame::ToWorld(const Vector3f & V) const
{
	return S * V.x() + T * V.y() + N * V.z();
}

float Frame::CosTheta(const Vector3f & V)
{
	return V.z();
}

float Frame::SinTheta(const Vector3f & V)
{
	float Temp = SinTheta2(V);
	if (Temp <= 0.0f)
	{
		return 0.0f;
	}
	return std::sqrt(Temp);
}

float Frame::TanTheta(const Vector3f & V)
{
	float Temp = 1.0f - V.z() * V.z();
	if (Temp <= 0.0f)
	{
		return 0.0f;
	}
	return std::sqrt(Temp) / V.z();
}

float Frame::SinTheta2(const Vector3f & V)
{
	return 1.0f - V.z() * V.z();
}

float Frame::CosTheta2(const Vector3f & V)
{
	return V.z() * V.z();
}

float Frame::SinPhi(const Vector3f & V)
{
	float Temp = SinTheta(V);
	if (Temp == 0.0f)
	{
		return 1.0f;
	}
	return Clamp(V.y() / Temp, -1.0f, 1.0f);
}

float Frame::CosPhi(const Vector3f & V)
{
	float Temp = Frame::SinTheta(V);
	if (Temp == 0.0f)
	{
		return 1.0f;
	}
	return Clamp(V.x() / Temp, -1.0f, 1.0f);
}

float Frame::SinPhi2(const Vector3f & V)
{
	return Clamp(V.y() * V.y() / SinTheta2(V), 0.0f, 1.0f);
}

float Frame::CosPhi2(const Vector3f & V)
{
	return Clamp(V.x() * V.x() / SinTheta2(V), 0.0f, 1.0f);
}

bool Frame::operator==(const Frame & Rhs) const
{
	return Rhs.S == S && Rhs.T == T && Rhs.N == N;
}

bool Frame::operator!=(const Frame & Rhs) const
{
	return !operator==(Rhs);
}

std::string Frame::ToString() const
{
	return tfm::format(
		"Frame[S = %s,\n  T = %s,\n  N = %s\n]", 
		S.ToString(), T.ToString(), N.ToString()
	);
}

NAMESPACE_END