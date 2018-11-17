#include <core\Emitter.hpp>

NAMESPACE_BEGIN

EmitterQueryRecord::EmitterQueryRecord()
{

}

EmitterQueryRecord::EmitterQueryRecord(const Point3f & Ref) :
	Ref(Ref)
{

}

EmitterQueryRecord::EmitterQueryRecord(const Emitter * pEmitter, const Point3f & Ref, const Point3f & P, const Normal3f & N) :
	pEmitter(pEmitter), Ref(Ref), P(P), N(N)
{
	Wi = P - Ref;
	Distance = Wi.norm();
	Wi /= Distance;
}

std::string EmitterQueryRecord::ToString() const
{
	return tfm::format(
		"EmitterQueryRecord[\n"
		"  emitter = \"%s\",\n"
		"  ref = %s,\n"
		"  p = %s,\n"
		"  n = %s,\n"
		"  pdf = %f,\n"
		"  wi = %s,\n"
		"  distance = %f\n"
		"]",
		Indent(pEmitter->ToString()),
		Ref.ToString(),
		P.ToString(),
		N.ToString(),
		Pdf,
		Wi.ToString(),
		Distance
	);
}

Object::EClassType Emitter::GetClassType() const
{
	return EClassType::EEmitter;
}

void Emitter::SetMesh(Mesh * pMesh)
{
	m_pMesh = pMesh;
}

EEmitterType Emitter::GetEmitterType() const
{
	return m_Type;
}

bool Emitter::IsDelta() const
{
	return m_Type == EEmitterType::EPoint;
}

NAMESPACE_END


