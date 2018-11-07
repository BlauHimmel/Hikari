#include <core\Acceleration.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(Acceleration, XML_ACCELERATION_BRUTO_LOOP);

Acceleration::Acceleration(const PropertyList & PropList)
{

}

void Acceleration::AddMesh(Mesh * pMesh)
{
	m_pMeshes.push_back(pMesh);
	m_BBox.ExpandBy(pMesh->GetBoundingBox());

	m_Primitives.reserve(m_Primitives.size() + pMesh->GetTriangleCount());
	MatrixXu Indices = pMesh->GetIndices();
	uint32_t * pData = Indices.data();
	for (std::ptrdiff_t i = 0; i < Indices.cols(); i++)
	{
		Primitive Tri;
		Tri.pMesh = pMesh;
		Tri.pFacet = pData + i * 3;
		Tri.iFacet = uint32_t(i);
		m_Primitives.push_back(Tri);
	}
}

void Acceleration::Build()
{
	/* Nothing to do here for now */
}

const BoundingBox3f & Acceleration::GetBoundingBox() const
{
	return m_BBox;
}

bool Acceleration::RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const
{
	bool bFoundIntersection = false;       // Was an intersection found so far?
	uint32_t iFacet = uint32_t(-1);        // Triangle index of the closest intersection

	Ray3f RayCopy(Ray); /// Make a copy of the ray (we will need to update its '.MaxT' value)

	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		Mesh * pMesh = m_pMeshes[i];
		/* Brute force search through all triangles */
		for (uint32_t j = 0; j < pMesh->GetTriangleCount(); ++j)
		{
			float U, V, T;
			if (pMesh->RayIntersect(j, RayCopy, U, V, T))
			{
				/* An intersection was found! Can terminate
				immediately if this is a shadow ray query */
				if (bShadowRay)
				{
					return true;
				}

				RayCopy.MaxT = Isect.T = T;
				Isect.UV = Point2f(U, V);
				Isect.pMesh = pMesh;
				iFacet = j;
				bFoundIntersection = true;
			}
		}
	}

	if (bFoundIntersection)
	{
		/* At this point, we now know that there is an intersection,
		and we know the triangle index of the closest such intersection.

		The following computes a number of additional properties which
		characterize the intersection (normals, texture coordinates, etc..)
		*/

		/* Find the barycentric coordinates */
		Vector3f Barycentric;
		Barycentric << 1 - Isect.UV.sum(), Isect.UV;

		/* References to all relevant mesh buffers */
		const Mesh * pMesh = Isect.pMesh;
		const MatrixXf & V = pMesh->GetVertexPositions();
		const MatrixXf & N = pMesh->GetVertexNormals();
		const MatrixXf & UV = pMesh->GetVertexTexCoords();
		const MatrixXu & F = pMesh->GetIndices();

		/* Vertex indices of the triangle */
		uint32_t Idx0 = F(0, iFacet), Idx1 = F(1, iFacet), Idx2 = F(2, iFacet);
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

	return bFoundIntersection;
}

Object::EClassType Acceleration::GetClassType() const
{
	return EClassType::EAcceleration;
}

std::string Acceleration::ToString() const
{
	return "BrutoLoop[]";
}

NAMESPACE_END
