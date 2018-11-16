#include <core\Mesh.hpp>
#include <core\Shape.hpp>

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
	ShadowRay.Origin = P + GeometricFrame.N * Epsilon;
	ShadowRay.Direction = Pt - ShadowRay.Origin;
	ShadowRay.MaxT = 1.0f;
	ShadowRay.MinT = 0.0f;
	ShadowRay.Update();
	return ShadowRay;
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

Triangle::Triangle()
{

}

Triangle::Triangle(Mesh * pMesh, uint32_t * pFacet, uint32_t iFacet) :
	m_pMesh(pMesh), m_pFacet(pFacet), m_iFacet(iFacet)
{

}

void Triangle::SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const
{
	float SqrOneMinusEpsilon1 = std::sqrt(1.0f - Sample.x());
	float Alpha = 1.0f - SqrOneMinusEpsilon1;
	float Beta = Sample.y() * SqrOneMinusEpsilon1;
	float Gamma = 1.0f - Alpha - Beta;

	const MatrixXu & Indices = m_pMesh->GetIndices();
	const MatrixXf & Positions = m_pMesh->GetVertexPositions();
	const MatrixXf & Normals = m_pMesh->GetVertexNormals();

	uint32_t Idx0 = Indices(0, m_iFacet), Idx1 = Indices(1, m_iFacet), Idx2 = Indices(2, m_iFacet);
	Point3f P0 = Positions.col(Idx0), P1 = Positions.col(Idx1), P2 = Positions.col(Idx2);

	P = Gamma * P0 + Alpha * P1 + Beta * P2;

	if (Normals.size() > 0)
	{
		Normal3f N0 = Normals.col(Idx0), N1 = Normals.col(Idx1), N2 = Normals.col(Idx2);
		N = Gamma * N0 + Alpha * N1 + Beta * N2;
	}
}

float Triangle::SurfaceArea() const
{
	return m_pMesh->SurfaceArea(m_iFacet);
}

BoundingBox3f Triangle::GetBoundingBox() const
{
	return m_pMesh->GetBoundingBox(m_iFacet);
}

Point3f Triangle::GetCentroid() const
{
	return m_pMesh->GetCentroid(m_iFacet);
}

bool Triangle::RayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const
{
	return m_pMesh->RayIntersect(m_iFacet, Ray, U, V, T);
}

Mesh * Triangle::GetMesh() const
{
	return m_pMesh;
}

uint32_t Triangle::GetFacetIndex() const
{
	return m_iFacet;
}

std::string Triangle::ToString() const
{
	const MatrixXf & V = m_pMesh->GetVertexPositions();
	uint32_t iV0 = m_pFacet[0];
	uint32_t iV1 = m_pFacet[1];
	uint32_t iV2 = m_pFacet[2];
	Point3f P0 = V.col(iV0);
	Point3f P1 = V.col(iV1);
	Point3f P2 = V.col(iV2);

	return tfm::format(
		"Triangle[\n"
		"  v0 = %s\n"
		"  v1 = %s\n"
		"  v2 = %s\n"
		"]",
		P0.ToString(),
		P1.ToString(),
		P2.ToString()
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
		m_pBSDF = (BSDF*)(ObjectFactory::CreateInstance(DEFAULT_MESH_BSDF, PropertyList()));
	}

	// Create the pdf (defined by the area of each triangle)
	for (uint32_t i = 0; i < GetTriangleCount(); i++)
	{
		float Area = SurfaceArea(i);
		m_PDF.Append(Area);
		m_MeshArea += Area;
	}
	m_PDF.Normalize();
}

uint32_t Mesh::GetTriangleCount() const
{
	return uint32_t(m_F.cols());
}

uint32_t Mesh::GetVertexCount() const
{
	return uint32_t(m_V.cols());
}

void Mesh::SamplePosition(float Sample1D, const Point2f & Sample2D, Point3f & P, Normal3f & N) const
{
	size_t TriangleIdx = m_PDF.Sample(Sample1D);

	float SqrOneMinusEpsilon1 = std::sqrt(1.0f - Sample2D.x());
	float Alpha = 1.0f - SqrOneMinusEpsilon1;
	float Beta = Sample2D.y() * SqrOneMinusEpsilon1;
	float Gamma = 1.0f - Alpha - Beta;

	uint32_t Idx0 = m_F(0, TriangleIdx), Idx1 = m_F(1, TriangleIdx), Idx2 = m_F(2, TriangleIdx);
	Point3f P0 = m_V.col(Idx0), P1 = m_V.col(Idx1), P2 = m_V.col(Idx2);

	P = Gamma * P0 + Alpha * P1 + Beta * P2;

	if (m_N.size() > 0)
	{
		Normal3f N0 = m_N.col(Idx0), N1 = m_N.col(Idx1), N2 = m_N.col(Idx2);
		N = Gamma * N0 + Alpha * N1 + Beta * N2;
	}
}

float Mesh::SurfaceArea() const
{
	return m_MeshArea;
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
