#include <core\Shape.hpp>

NAMESPACE_BEGIN

Mesh * Shape::GetMesh() const
{
	return nullptr;
}

uint32_t Shape::GetFacetIndex() const
{
	return uint32_t(-1);
}

bool Shape::IsEmitter() const
{
	return false;
}

Emitter * Shape::GetEmitter()
{
	return nullptr;
}

const Emitter * Shape::GetEmitter() const
{
	return nullptr;
}

void Shape::ComputeCurvature(const Intersection & Isect, float & H, float & K) const
{
	if (!Isect.bHasUVPartial)
	{
		H = 0.0f;
		K = 0.0f;
		LOG(ERROR) << "Curvature cannot be computed, since there is no derivative information.";
		return;
	}
	/* Compute the coefficients of the first and second fundamental form */
	float E = Isect.dPdU.dot(Isect.dPdU);
	float F = Isect.dPdU.dot(Isect.dPdV);
	float G = Isect.dPdV.dot(Isect.dPdV);
	float L = -Isect.dPdU.dot(Isect.dNdU);
	float M = -Isect.dPdV.dot(Isect.dNdU);
	float N = -Isect.dPdV.dot(Isect.dNdV);

	float InvDenom = 1.0f / (E * G - F * F);
	K = (L * N - M * M) * InvDenom;
	H = 0.5f * (L * G - 2.0f * M * F + N * E) * InvDenom;
}

Object::EClassType Shape::GetClassType() const
{
	return EClassType::EShape;
}

NAMESPACE_END