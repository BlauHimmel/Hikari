#include <acceleration\BVHAcceleration.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(BVHAcceleration, XML_ACCELERATION_BVH);

struct BVHTraversal
{
	uint32_t Idx;
	float MinT;
	BVHTraversal() { Idx = 0; MinT = std::numeric_limits<float>::max(); }
	BVHTraversal(uint32_t Idx, float MinT) : Idx(Idx), MinT(MinT) { }
};

struct BVHBuildEntry
{
	uint32_t iParent = 0;
	uint32_t iStart = 0, iEnd = 0;
};

BVHAcceleration::BVHAcceleration(const PropertyList & PropList) : 
	Acceleration(PropList)
{
	m_LeafSize = uint32_t(PropList.GetInteger(XML_ACCELERATION_BVH_LEAF_SIZE, DEFAULT_ACCELERATION_BVH_LEAF_SIZE));
}

BVHAcceleration::~BVHAcceleration()
{
	delete[] m_pFlatTree;
}

void BVHAcceleration::Build()
{
	TIMER_START(TimeBVHBuild);

	const uint32_t STACK_MAX_SIZE = 1024;
	const uint32_t UNTOUCHED = 0xffffffff;
	const uint32_t TOUCHED_TWICE = 0xfffffffd;
	BVHBuildEntry Stack[STACK_MAX_SIZE];
	uint32_t iStackPtr = 0;

	// Push the root node
	Stack[iStackPtr].iStart = 0;
	Stack[iStackPtr].iEnd = m_Primitives.size();
	Stack[iStackPtr].iParent = 0xfffffffc;
	iStackPtr++;

	std::vector<BVHFlatNode> BuildNodes;
	BuildNodes.reserve(m_Primitives.size() * 2);

	while (iStackPtr > 0)
	{
		BVHBuildEntry & BuildEntry = (Stack[--iStackPtr]);
		uint32_t iStart = BuildEntry.iStart;
		uint32_t iEnd = BuildEntry.iEnd;
		uint32_t nPrimitives = iEnd - iStart;

		m_nNodes++;

		BVHFlatNode FlatNode;
		FlatNode.iStart = iStart;
		FlatNode.nPrimitives = nPrimitives;
		FlatNode.nRightChildOffset = UNTOUCHED;

		// Calculate the bounding box for this node
		Primitive & Prim = m_Primitives[iStart];
		Mesh * pMesh = Prim.pMesh;
		uint32_t iFacet = Prim.iFacet;
		BoundingBox3f BBox = pMesh->GetBoundingBox(iFacet);
		BoundingBox3f Centriod = pMesh->GetCentroid(iFacet);

		for (uint32_t i = iStart + 1; i < iEnd; i++)
		{
			Prim = m_Primitives[i];
			pMesh = Prim.pMesh;
			iFacet = Prim.iFacet;
			BBox.ExpandBy(pMesh->GetBoundingBox(iFacet));
			Centriod.ExpandBy(pMesh->GetCentroid(iFacet));
		}

		FlatNode.BBox = BBox;

		// If the number of primitives at this point is less than the leaf
		// size, then this will become a leaf. (Signified by rightOffset == 0)
		if (nPrimitives <= m_LeafSize)
		{
			FlatNode.nRightChildOffset = 0;
			m_nLeafs++;
		}

		BuildNodes.push_back(FlatNode);

		// Child touches parent
		// Special case: Don't do this for the root.
		if (BuildEntry.iParent != 0xfffffffc)
		{
			BuildNodes[BuildEntry.iParent].nRightChildOffset--;

			// When this is the second touch, this is the right child.
			// The right child sets up the offset for the flat tree.
			if (BuildNodes[BuildEntry.iParent].nRightChildOffset == TOUCHED_TWICE)
			{
				BuildNodes[BuildEntry.iParent].nRightChildOffset = m_nNodes - 1 - BuildEntry.iParent;
			}
		}

		// If this is a leaf, no need to subdivide.
		if (FlatNode.nRightChildOffset == 0)
		{
			continue;
		}

		// Set the split dimensions
		uint32_t SplitDim = Centriod.GetMajorAxis();

		// Split on the center of the longest axis
		float SplitCoord = 0.5f * (Centriod.Min[SplitDim] + Centriod.Max[SplitDim]);

		// Partition the list of objects on this split
		uint32_t iMid = iStart;
		for (uint32_t i = iStart + 1; i < iEnd; i++)
		{
			Prim = m_Primitives[i];
			pMesh = Prim.pMesh;
			iFacet = Prim.iFacet;

			if (pMesh->GetCentroid(iFacet)[SplitDim] < SplitCoord)
			{
				std::swap(m_Primitives[i], m_Primitives[iMid]);
				iMid++;
			}
		}

		// If we get a bad split, just choose the center...
		if (iMid == iStart || iMid == iEnd)
		{
			iMid = iStart + (iEnd - iStart) / 2;
		}

		// Right child pushed
		Stack[iStackPtr].iStart = iMid;
		Stack[iStackPtr].iEnd = iEnd;
		Stack[iStackPtr].iParent = m_nNodes - 1;
		iStackPtr++;

		// Left child pushed
		Stack[iStackPtr].iStart = iStart;
		Stack[iStackPtr].iEnd = iMid;
		Stack[iStackPtr].iParent = m_nNodes - 1;
		iStackPtr++;
	}

	m_pFlatTree = new BVHFlatNode[m_nNodes];
	memcpy(m_pFlatTree, BuildNodes.data(), m_nNodes * sizeof(BVHFlatNode));

	TIMER_END(TimeBVHBuild);
	LOG(INFO) << "Build BVH (" << m_nNodes << " nodes, with " << m_nLeafs << " leafs) in " << TimeBVHBuild << " sec.";
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
