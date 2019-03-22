#include <core\Intersection.hpp>
#include <core\Shape.hpp>
#include <core\Ray.hpp>

NAMESPACE_BEGIN

Intersection::Intersection() { }

Vector3f Intersection::ToLocal(const Vector3f & Dir) const
{
	return ShadingFrame.ToLocal(Dir);
}

Vector3f Intersection::ToWorld(const Vector3f & Dir) const
{
	return ShadingFrame.ToWorld(Dir);
}

Ray3f Intersection::SpawnShadowRay(const Point3f & Pt) const
{
	Ray3f ShadowRay;
	ShadowRay.Origin = P;
	ShadowRay.Direction = Pt - ShadowRay.Origin;
	ShadowRay.MaxT = 1.0f - float(Epsilon);
	ShadowRay.MinT = float(Epsilon);
	ShadowRay.Update();
	return ShadowRay;
}

void Intersection::ComputeScreenSpacePartial(const Ray3f & Ray)
{
	/* Compute the texture coordinates partials wrt.
	changes in the screen-space position. Based on PBRT 10.1.1 */

	if (!Ray.bHasDifferentials || !bHasUVPartial)
	{
		return;
	}

	if (dPdU.isZero() && dPdV.isZero())
	{
		dUdX = 0.0f;
		dUdY = 0.0f;
		dVdX = 0.0f;
		dVdY = 0.0f;
		return;
	}

	/* Compute a few projections onto the surface normal */
	float pP = GeometricFrame.N.dot(Vector3f(P));
	float pRxOrigin = GeometricFrame.N.dot(Vector3f(Ray.RxOrigin));
	float pRyOrigin = GeometricFrame.N.dot(Vector3f(Ray.RyOrigin));
	float pRxDirection = GeometricFrame.N.dot(Ray.RxDirection);
	float pRyDirection = GeometricFrame.N.dot(Ray.RyDirection);

	if (pRxDirection == 0.0 || pRyDirection == 0.0)
	{
		dUdX = 0.0f;
		dUdY = 0.0f;
		dVdX = 0.0f;
		dVdY = 0.0f;
		return;
	}

	/* Compute ray-plane intersections against the offset rays */
	float Tx = (pP - pRxOrigin) / pRxDirection;
	float Ty = (pP - pRyOrigin) / pRyDirection;

	/* Calculate the U and V partials by solving two out
	of a set of 3 equations in an overconstrained system */
	float AbsX = std::abs(GeometricFrame.N.x());
	float AbsY = std::abs(GeometricFrame.N.y());
	float AbsZ = std::abs(GeometricFrame.N.z());

	float A[2][2], Bx[2], By[2], X[2];
	int Axes[2];

	if (AbsX > AbsY && AbsX > AbsZ)
	{
		Axes[0] = 1; Axes[1] = 2;
	}
	else if (AbsY > AbsZ)
	{
		Axes[0] = 0; Axes[1] = 2;
	}
	else
	{
		Axes[0] = 0; Axes[1] = 1;
	}

	A[0][0] = dPdU[Axes[0]];
	A[0][1] = dPdV[Axes[0]];
	A[1][0] = dPdU[Axes[1]];
	A[1][1] = dPdV[Axes[1]];

	/* Auxilary intersection point of the adjacent rays */
	Point3f Px = Ray.RxOrigin + Ray.RxDirection * Tx;
	Point3f	Py = Ray.RyOrigin + Ray.RyDirection * Ty;

	Bx[0] = Px[Axes[0]] - P[Axes[0]];
	Bx[1] = Px[Axes[1]] - P[Axes[1]];
	By[0] = Py[Axes[0]] - P[Axes[0]];
	By[1] = Py[Axes[1]] - P[Axes[1]];

	if (SolveLinearSystem2x2(A, Bx, X))
	{
		dUdX = X[0];
		dVdX = X[1];
	}
	else
	{
		dUdX = 1.0f;
		dVdX = 0.0f;
	}

	if (SolveLinearSystem2x2(A, By, X))
	{
		dUdY = X[0];
		dVdY = X[1];
	}
	else
	{
		dUdY = 0.0f;
		dVdY = 1.0f;
	}
}

std::string Intersection::ToString() const
{
	if (pShape == nullptr)
	{
		return "Intersection[invalid]";
	}


	if (!bHasUVPartial)
	{
		return tfm::format(
			"Intersection[\n"
			"  p = %s,\n"
			"  t = %f,\n"
			"  uv = %s,\n"
			"  shadingFrame = %s,\n"
			"  geometricFrame = %s,\n"
			"  shape = %s\n"
			"]",
			P.ToString(),
			T,
			UV.ToString(),
			Indent(ShadingFrame.ToString()),
			Indent(GeometricFrame.ToString()),
			pShape->ToString()
		);
	}
	else
	{
		std::string Str1 = tfm::format(
			"Intersection[\n"
			"  p = %s,\n"
			"  t = %f,\n"
			"  uv = %s,\n"
			"  shadingFrame = %s,\n"
			"  geometricFrame = %s,\n"
			"  shape = %s,\n",
			P.ToString(),
			T,
			UV.ToString(),
			Indent(ShadingFrame.ToString()),
			Indent(GeometricFrame.ToString()),
			pShape->ToString()
		);

		std::string Str2 = tfm::format(
			"  dPdU = %s,\n",
			"  dPdV = %s,\n",
			"  dNdU = %s,\n",
			"  dNdV = %s,\n",
			"  dUdX = %f,\n",
			"  dUdY = %f,\n",
			"  dVdX = %f,\n",
			"  dVdY = %f\n",
			"]",
			dPdU.ToString(),
			dPdV.ToString(),
			dNdU.ToString(),
			dNdV.ToString(),
			dUdX,
			dUdY,
			dVdX,
			dVdY
		);

		return Str1 + Str2;
	}
}

NAMESPACE_END