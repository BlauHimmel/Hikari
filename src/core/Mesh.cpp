#include <core\Mesh.hpp>

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

std::string Intersection::ToString() const
{
	if (pMesh == nullptr)
	{
		return "Intersection[invalid]";
	}

	return tfm::format(
		"Intersection[\n"
		"  p = %s,\n"
		"  t = %f,\n"
		"  uv = %s,\n"
		"  shadingFrame = %s,\n"
		"  geometricFrame = %s,\n"
		"  mesh = %s\n"
		"]",
		P.ToString(),
		T,
		UV.ToString(),
		Indent(ShadingFrame.ToString()),
		Indent(GeometricFrame.ToString()),
		pMesh->ToString()
	);
}

Mesh::~Mesh()
{
	delete m_pBSDF;
	delete m_pEmitter;
}

void Mesh::Activate()
{
	if (m_pBSDF == nullptr)
	{
		/* If no material was assigned, instantiate a diffuse BRDF */
		LOG(WARNING) << "No BSDF was specified, create a default BSDF.";
		m_pBSDF = (BSDF*)(ObjectFactory::CreateInstance(XML_BSDF_DIFFUSE, PropertyList()));
	}
}

uint32_t Mesh::GetTriangleCount() const
{
	return uint32_t(m_F.cols());
}

uint32_t Mesh::GetVertexCount() const
{
	return uint32_t(m_V.cols());
}

void Mesh::SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const
{
	throw HikariException("Mesh::SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const is not yet implemented!");
}

float Mesh::SurfaceArea(uint32_t Index) const
{
	uint32_t Idx0 = m_F(0, Index), Idx1 = m_F(1, Index), Idx2 = m_F(2, Index);
	const Point3f P0 = m_V.col(Idx0), P1 = m_V.col(Idx1), P2 = m_V.col(Idx2);
	return 0.5f * Vector3f((P1 - P0).cross(P2 - P0)).norm();
}

const BoundingBox3f & Mesh::GetBoundingBox() const
{
	return m_BBox;
}

BoundingBox3f Mesh::GetBoundingBox(uint32_t Index) const
{
	BoundingBox3f Result(m_V.col(m_F(0, Index)));
	Result.ExpandBy(m_V.col(m_F(1, Index)));
	Result.ExpandBy(m_V.col(m_F(2, Index)));
	return Result;
}

Point3f Mesh::GetCentroid(uint32_t Index) const
{
	return (1.0f / 3.0f) * (m_V.col(m_F(0, Index)) + m_V.col(m_F(1, Index)) + m_V.col(m_F(2, Index)));
}

bool Mesh::RayIntersect(uint32_t Index, const Ray3f & Ray, float & U, float & V, float & T) const
{
	uint32_t Idx0 = m_F(0, Index), Idx1 = m_F(1, Index), Idx2 = m_F(2, Index);
	const Point3f P0 = m_V.col(Idx0), P1 = m_V.col(Idx1), P2 = m_V.col(Idx2);

	/* Find vectors for two edges sharing v[0] */
	Vector3f Edge1 = P1 - P0, Edge2 = P2 - P0;

	/* Begin calculating determinant - also used to calculate U parameter */
	Vector3f PVec = Ray.Direction.cross(Edge2);

	/* If determinant is near zero, ray lies in plane of triangle */
	float Det = Edge1.dot(PVec);

	if (Det > -1e-8f && Det < 1e-8f)
	{
		return false;
	}
	float InvDet = 1.0f / Det;

	/* Calculate distance from v[0] to ray origin */
	Vector3f TVec = Ray.Origin - P0;

	/* Calculate U parameter and test bounds */
	U = TVec.dot(PVec) * InvDet;
	if (U < 0.0 || U > 1.0)
	{
		return false;
	}

	/* Prepare to test V parameter */
	Vector3f QVec = TVec.cross(Edge1);

	/* Calculate V parameter and test bounds */
	V = Ray.Direction.dot(QVec) * InvDet;
	if (V < 0.0 || U + V > 1.0)
	{
		return false;
	}

	/* Ray intersects triangle -> compute t */
	T = Edge2.dot(QVec) * InvDet;

	return T >= Ray.MinT && T <= Ray.MaxT;
}

const MatrixXf & Mesh::GetVertexPositions() const
{
	return m_V;
}

const MatrixXf & Mesh::GetVertexNormals() const
{
	return m_N;
}

const MatrixXf & Mesh::GetVertexTexCoords() const
{
	return m_UV;
}

const MatrixXu & Mesh::GetIndices() const
{
	return m_F;
}

bool Mesh::IsEmitter() const
{
	return m_pEmitter != nullptr;
}

Emitter * Mesh::GetEmitter()
{
	return m_pEmitter;
}

const Emitter * Mesh::GetEmitter() const
{
	return m_pEmitter;
}

const BSDF * Mesh::GetBSDF() const
{
	return m_pBSDF;
}

const std::string & Mesh::GetName() const
{
	return m_Name;
}

void Mesh::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EBSDF:
		if (m_pBSDF != nullptr)
		{
			throw HikariException("Mesh: tried to register multiple BSDF instances!");
		}
		m_pBSDF = (BSDF*)(pChildObj);
		break;
	case EClassType::EEmitter: 
		if (m_pEmitter != nullptr)
		{
			throw HikariException("Mesh: tried to register multiple Emitter instances!");
		}
		m_pEmitter = (Emitter*)(pChildObj);
		break;
	default:
		throw HikariException("Mesh::AddChild(<%s>) is not supported!", ClassTypeName(pChildObj->GetClassType()));
	}
}

std::string Mesh::ToString() const
{
	return tfm::format(
		"Mesh[\n"
		"  name = \"%s\",\n"
		"  vertexCount = %i,\n"
		"  triangleCount = %i,\n"
		"  bsdf = %s,\n"
		"  emitter = %s\n"
		"]",
		m_Name,
		m_V.cols(),
		m_F.cols(),
		m_pBSDF != nullptr ? Indent(m_pBSDF->ToString()) : std::string("null"),
		m_pEmitter != nullptr ? Indent(m_pEmitter->ToString()) : std::string("null")
	);
}

Object::EClassType Mesh::GetClassType() const
{
	return EClassType::EMesh;
}

Mesh::Mesh() { }

NAMESPACE_END