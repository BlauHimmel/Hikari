#include <texture\WireframeTexture.hpp>
#include <core\Intersection.hpp>
#include <core\Shape.hpp>
#include <core\Mesh.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(WireframeTexture, XML_TEXTURE_WIREFRAME);

WireframeTexture::WireframeTexture(const PropertyList & PropList)
{
	m_InteriorColor = PropList.GetColor(XML_TEXTURE_WIREFRAME_INTERIOR_COLOR, DEFAULT_TEXTURE_WIREFRAME_INTERIOR_COLOR);
	m_EdgeColor = PropList.GetColor(XML_TEXTURE_WIREFRAME_EDGE_COLOR, DEFAULT_TEXTURE_WIREFRAME_EDGE_COLOR);
	m_EdgeWidth = PropList.GetFloat(XML_TEXTURE_WIREFRAME_EDGE_WIDTH, DEFAULT_TEXTURE_WIREFRAME_EDGE_WIDTH);
	m_TransitionWidth = PropList.GetFloat(XML_TEXTURE_WIREFRAME_TRANSITION_WIDTH, DEFAULT_TEXTURE_WIREFRAME_TRANSITION_WIDTH);
	m_UVOffset[0] = PropList.GetFloat(XML_TEXTURE_WIREFRAME_OFFSE_U, DEFAULT_TEXTURE_WIREFRAME_OFFSET_U);
	m_UVOffset[1] = PropList.GetFloat(XML_TEXTURE_WIREFRAME_OFFSE_V, DEFAULT_TEXTURE_WIREFRAME_OFFSET_V);
	m_UVScale[0] = PropList.GetFloat(XML_TEXTURE_WIREFRAME_SCALE_U, DEFAULT_TEXTURE_WIREFRAME_SCALE_U);
	m_UVScale[1] = PropList.GetFloat(XML_TEXTURE_WIREFRAME_SCALE_V, DEFAULT_TEXTURE_WIREFRAME_SCALE_V);
}

Color3f WireframeTexture::Eval(const Intersection & Isect, bool bFilter) const
{
	const Mesh * pMesh = Isect.pShape->GetMesh();
	if (pMesh == nullptr)
	{
		return m_InteriorColor;
	}

	if (Isect.pShape->GetFacetIndex() >= pMesh->GetTriangleCount())
	{
		return m_InteriorColor;
	}

	const MatrixXf & Positions = pMesh->GetVertexPositions();
	const MatrixXu & Indices = pMesh->GetIndices();

	/* Automatically find the best edge width, i.e. 0.1 * Avg(EdgeLength) */
	if (m_EdgeWidth == 0.0f)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_EdgeWidth == 0.0f)
		{
			float EdgeWidth = 0.0f;
			for (uint32_t i = 0; i < pMesh->GetTriangleCount(); i++)
			{
				uint32_t Idx0 = Indices(0, i), Idx1 = Indices(1, i), Idx2 = Indices(2, i);
				Point3f P0 = Positions.col(Idx0), P1 = Positions.col(Idx1), P2 = Positions.col(Idx2);
				float L0 = (P1 - P0).norm(), L1 = (P2 - P1).norm(), L2 = (P0 - P2).norm();
				EdgeWidth += (L0 + L1 + L2);
			}
			m_EdgeWidth = 0.1f * EdgeWidth / (3 * pMesh->GetTriangleCount());
		}
	}

	float MinDistantceSq = std::numeric_limits<float>::max();

	uint32_t iFacet = Isect.pShape->GetFacetIndex();
	uint32_t Idx0 = Indices(0, iFacet), Idx1 = Indices(1, iFacet), Idx2 = Indices(2, iFacet);
	Point3f Ps[] = { Positions.col(Idx0), Positions.col(Idx1), Positions.col(Idx2) };

	for (uint32_t i = 0; i < 3; i++)
	{
		const Point3f & Current = Ps[i];
		const Point3f & Next = Ps[(i + 1) % 3];

		Vector3f D1 = (Next - Current).normalized();
		Vector3f D2 = Isect.P - Current;

		/* Distance from Isect.P to the edge Current-Next */
		float DistantceSq = (Current + D1 * D1.dot(D2) - Isect.P).squaredNorm();
		MinDistantceSq = std::min(MinDistantceSq, DistantceSq);
	}

	float Alpha = SmoothStep(m_EdgeWidth * (1.0f - m_TransitionWidth), m_EdgeWidth, std::sqrt(MinDistantceSq));
	return m_EdgeColor * (1.0f - Alpha) + m_InteriorColor * Alpha;
}

Color3f WireframeTexture::GetAverage() const
{
	/* Hard to approximate */
	return (m_EdgeColor + m_InteriorColor) * 0.5f;
}

Color3f WireframeTexture::GetMinimum() const
{
	Color3f Min;
	Min[0] = std::min(m_EdgeColor[0], m_InteriorColor[0]);
	Min[1] = std::min(m_EdgeColor[1], m_InteriorColor[1]);
	Min[2] = std::min(m_EdgeColor[2], m_InteriorColor[2]);
	return Min;
}

Color3f WireframeTexture::GetMaximum() const
{
	Color3f Max;
	Max[0] = std::max(m_EdgeColor[0], m_InteriorColor[0]);
	Max[1] = std::max(m_EdgeColor[1], m_InteriorColor[1]);
	Max[2] = std::max(m_EdgeColor[2], m_InteriorColor[2]);
	return Max;
}

Vector3i WireframeTexture::GetDimension() const
{
	LOG(WARNING) << "WireframeTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool WireframeTexture::IsConstant() const
{
	return false;
}

bool WireframeTexture::IsMonochromatic() const
{
	return m_EdgeColor[0] == m_EdgeColor[1] && m_EdgeColor[0] == m_EdgeColor[2] &&
		m_InteriorColor[0] == m_InteriorColor[1] && m_InteriorColor[0] == m_InteriorColor[2];
}

std::string WireframeTexture::ToString() const
{
	return tfm::format(
		"WireframeTexture[\n"
		"  interiorColor = %s,\n"
		"  edgeColor = %s,\n"
		"  edgeWidth = %f,\n"
		"  transitionWidth = %f,\n"
		"]",
		m_InteriorColor.ToString(),
		m_EdgeColor.ToString(),
		m_EdgeWidth,
		m_TransitionWidth
	);
}

NAMESPACE_END


