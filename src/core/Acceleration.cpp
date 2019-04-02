#include <core\Acceleration.hpp>
#include <core\Shape.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(Acceleration, XML_ACCELERATION_BRUTO_LOOP);

Acceleration::Acceleration(const PropertyList & PropList)
{

}

void Acceleration::AddMesh(Mesh * pMesh)
{
	m_BBox.ExpandBy(pMesh->GetBoundingBox());

	m_pShapes.reserve(m_pShapes.size() + pMesh->GetTriangleCount());
	MatrixXu Indices = pMesh->GetIndices();
	uint32_t * pData = Indices.data();
	Triangle * pTri = m_MemoryArena.Alloc<Triangle>(Indices.cols());
	for (std::ptrdiff_t i = 0; i < Indices.cols(); i++)
	{
		pTri[i].m_pMesh = pMesh;
		pTri[i].m_pFacet = pData + i * 3;
		pTri[i].m_iFacet = uint32_t(i);
		m_pShapes.push_back((Shape*)(&pTri[i]));
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

size_t Acceleration::GetUsedMemoryForShape() const
{
	return m_MemoryArena.TotalAllocated();
}

bool Acceleration::RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const
{
	bool bFoundIntersection = false;       // Was an intersection found so far?
	Shape * pFoundShape = nullptr;

	Ray3f RayCopy(Ray); /// Make a copy of the ray (we will need to update its '.MaxT' value)

	for (size_t i = 0; i < m_pShapes.size(); i++)
	{
		float U, V, T;
		if (m_pShapes[i]->RayIntersect(RayCopy, U, V, T))
		{
			/* An intersection was found! Can terminate
			immediately if this is a shadow ray query */
			if (bShadowRay)
			{
				return true;
			}

			RayCopy.MaxT = Isect.T = T;
			Isect.UV = Point2f(U, V);
			Isect.pShape = m_pShapes[i];

			pFoundShape = m_pShapes[i];
			bFoundIntersection = true;
		}
	}

	if (bFoundIntersection)
	{
		pFoundShape->PostIntersect(Isect);
		Isect.ComputeScreenSpacePartial(Ray);
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
