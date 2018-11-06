#include <acceleration\BVHAcceleration.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(BVHAcceleration, XML_ACCELERATION_BVH);

struct BVHTraversal
{
	uint32_t Idx;
	float MinT;
	BVHTraversal() { }
	BVHTraversal(uint32_t Idx, float MinT) : Idx(Idx), MinT(MinT) { }
};

struct BVHBuildEntry
{
	uint32_t iParent;
	uint32_t iStart, iEnd;
};

BVHAcceleration::BVHAcceleration(const PropertyList & PropList) : 
	Acceleration(PropList), m_nNodes(0), m_nLeafs(0), m_pFlatTree(nullptr)
{
	m_LeafSize = uint32_t(PropList.GetInteger(XML_ACCELERATION_BVH_LEAF_SIZE, DEFAULT_ACCELERATION_BVH_LEAF_SIZE));
}

BVHAcceleration::~BVHAcceleration()
{
	delete[] m_pFlatTree;
}

void BVHAcceleration::Build()
{
}

bool BVHAcceleration::RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const
{
	return false;
}

std::string BVHAcceleration::ToString() const
{
	return tfm::format(
		"BVHAcceleration[\n"
		"  leafSize = %s,\n"
		"  node = %s,\n"
		"  leafNode = %f,\n"
		"]",
		m_LeafSize,
		m_nNodes,
		m_nLeafs
	);
}

NAMESPACE_END
