#include <core\Mesh.hpp>
#include <core\Shape.hpp>

NAMESPACE_BEGIN

Triangle::Triangle()
{

}

Triangle::Triangle(Mesh * pMesh, uint32_t * pFacet, uint32_t iFacet) :
	m_pMesh(pMesh), m_pFacet(pFacet), m_iFacet(iFacet)
{

}

void Triangle::SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const
{
	// Ref : https://blog.csdn.net/noahzuo/article/details/52886447 or <<Graphics Gems>>
	float SqrY = Sample.y();
	float Alpha = 1.0f - SqrY;
	float Beta = (1.0f - Sample.x()) * SqrY;
	float Gamma = Sample.x() * SqrY;

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

void Triangle::PostIntersect(Intersection & Isect)
{
	/* At this point, we now know that there is an intersection,
	and we know the triangle index of the closest such intersection.

	The following computes a number of additional properties which
	characterize the intersection (normals, texture coordinates, etc..)
	*/

	Isect.pEmitter = Isect.pShape->GetEmitter();
	Isect.pBSDF = Isect.pShape->GetBSDF();

	/* Find the barycentric coordinates */
	Vector3f Barycentric;
	Barycentric << 1 - Isect.UV.sum(), Isect.UV;

	/* References to all relevant mesh buffers */
	const Mesh * pMesh = Isect.pShape->GetMesh();
	const MatrixXf & V = pMesh->GetVertexPositions();
	const MatrixXf & N = pMesh->GetVertexNormals();
	const MatrixXf & UV = pMesh->GetVertexTexCoords();
	const MatrixXu & F = pMesh->GetIndices();

	/* Vertex indices of the triangle */
	uint32_t Idx0 = F(0, m_iFacet), Idx1 = F(1, m_iFacet), Idx2 = F(2, m_iFacet);
	Point3f P0 = V.col(Idx0), P1 = V.col(Idx1), P2 = V.col(Idx2);

	/* Compute the intersection positon accurately using barycentric coordinates */
	Isect.P = Barycentric.x() * P0 + Barycentric.y() * P1 + Barycentric.z() * P2;

	/* Compute proper texture coordinates if provided by the mesh */
	if (UV.size() > 0)
	{
		Isect.UV = Barycentric.x() * UV.col(Idx0) + Barycentric.y() * UV.col(Idx1) + Barycentric.z() * UV.col(Idx2);
	}

	/* Compute the geometry frame */
	Isect.GeometricFrame = Frame((P1 - P0).cross(P2 - P0).normalized());

	if (N.size() > 0)
	{
		/* Compute the shading frame. Note that for simplicity,
		the current implementation doesn't attempt to provide
		tangents that are continuous across the surface. That
		means that this code will need to be modified to be able
		use anisotropic BRDFs, which need tangent continuity */

		Isect.ShadingFrame = Frame(
			(Barycentric.x() * N.col(Idx0) + Barycentric.y() * N.col(Idx1) + Barycentric.z() * N.col(Idx2)).normalized()
		);
	}
	else
	{
		Isect.ShadingFrame = Isect.GeometricFrame;
	}
}

Mesh * Triangle::GetMesh() const
{
	return m_pMesh;
}

uint32_t Triangle::GetFacetIndex() const
{
	return m_iFacet;
}

bool Triangle::IsEmitter() const
{
	return m_pMesh->IsEmitter();
}

Emitter * Triangle::GetEmitter()
{
	return m_pMesh->GetEmitter();
}

const Emitter * Triangle::GetEmitter() const
{
	return m_pMesh->GetEmitter();
}

const BSDF * Triangle::GetBSDF() const
{
	return m_pMesh->GetBSDF();
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
		"  v0 = %s, v1 = %s, v2 = %s,\n"
		"  emmiter = %s,\n"
		"  BSDF = %s"
		"]",
		P0.ToString(),
		P1.ToString(),
		P2.ToString(),
		IsEmitter() ? Indent(GetEmitter()->ToString()) : "<null>",
		Indent(GetBSDF()->ToString())
	);
}

Mesh::~Mesh()
{
	delete m_pBSDF;
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
	m_InvMeshArea = 1.0f / m_MeshArea;
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

	// Ref : https://blog.csdn.net/noahzuo/article/details/52886447 or <<Graphics Gems>>
	float SqrY = Sample2D.y();
	float Alpha = 1.0f - SqrY;
	float Beta = (1.0f - Sample2D.x()) * SqrY;
	float Gamma = Sample2D.x() * SqrY;

	uint32_t Idx0 = m_F(0, TriangleIdx), Idx1 = m_F(1, TriangleIdx), Idx2 = m_F(2, TriangleIdx);
	Point3f P0 = m_V.col(Idx0), P1 = m_V.col(Idx1), P2 = m_V.col(Idx2);

	P = Gamma * P0 + Alpha * P1 + Beta * P2;

	if (m_N.size() > 0)
	{
		Normal3f N0 = m_N.col(Idx0), N1 = m_N.col(Idx1), N2 = m_N.col(Idx2);
		N = Gamma * N0 + Alpha * N1 + Beta * N2;
	}
	else
	{
		N = (P1 - P0).cross(P2 - P0).normalized();
	}
}

float Mesh::Pdf() const
{
	return m_InvMeshArea;
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
