#include <acceleration\BVHAcceleration.hpp>
#include <core\Timer.hpp>
#include <core\Shape.hpp>

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

struct BVHBucket
{
	uint32_t nShape = 0;
	BoundingBox3f BBox;
};

BVHAcceleration::BVHAcceleration(const PropertyList & PropList) : 
	Acceleration(PropList)
{
	m_LeafSize = uint32_t(PropList.GetInteger(XML_ACCELERATION_BVH_LEAF_SIZE, DEFAULT_ACCELERATION_BVH_LEAF_SIZE));
	m_SplitMethod = PropList.GetString(XML_ACCELERATION_BVH_SPLIT_METHOD, DEFAULT_ACCELERATION_BVH_SPLIT_METHOD);
	if (m_SplitMethod != XML_ACCELERATION_BVH_SPLIT_METHOD_CENTER && m_SplitMethod != XML_ACCELERATION_BVH_SPLIT_METHOD_SAH)
	{
		LOG(WARNING) << "No split method \"" << m_SplitMethod << "\", use default split method";
		m_SplitMethod = DEFAULT_ACCELERATION_BVH_SPLIT_METHOD;
	}
}

BVHAcceleration::~BVHAcceleration()
{
	delete[] m_pFlatTree;
}

void BVHAcceleration::Build()
{
	Timer BVHBuildTimer;

	const uint32_t STACK_MAX_SIZE = 1024;
	const uint32_t UNTOUCHED = 0xffffffff;
	const uint32_t TOUCHED_TWICE = 0xfffffffd;
	BVHBuildEntry Stack[STACK_MAX_SIZE];
	uint32_t iStackPtr = 0;

	// Push the root node
	Stack[iStackPtr].iStart = 0;
	Stack[iStackPtr].iEnd = uint32_t(m_pShapes.size());
	Stack[iStackPtr].iParent = 0xfffffffc;
	iStackPtr++;

	std::vector<BVHFlatNode> BuildNodes;
	BuildNodes.reserve(m_pShapes.size() * 2);

	while (iStackPtr > 0)
	{
		const BVHBuildEntry & BuildEntry = (Stack[--iStackPtr]);
		uint32_t iStart = BuildEntry.iStart;
		uint32_t iEnd = BuildEntry.iEnd;
		uint32_t nShapes = iEnd - iStart;

		m_nNodes++;

		BVHFlatNode FlatNode;
		FlatNode.iStart = iStart;
		FlatNode.nShapes = nShapes;
		FlatNode.nRightChildOffset = UNTOUCHED;

		// Calculate the bounding box for this node
		BoundingBox3f BBox = m_pShapes[iStart]->GetBoundingBox();
		BoundingBox3f Centroid = m_pShapes[iStart]->GetCentroid();

		for (uint32_t i = iStart + 1; i < iEnd; i++)
		{
			BBox.ExpandBy(m_pShapes[i]->GetBoundingBox());
			Centroid.ExpandBy(m_pShapes[i]->GetCentroid());
		}

		FlatNode.BBox = BBox;

		// If the number of shapes at this point is less than the leaf
		// size, then this will become a leaf. (Signified by rightOffset == 0)
		if (nShapes <= m_LeafSize)
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
		uint32_t SplitDim = Centroid.GetMajorAxis();
		uint32_t iMid = iStart;

		if (m_SplitMethod == XML_ACCELERATION_BVH_SPLIT_METHOD_CENTER)
		{
			// Split on the center of the longest axis
			float SplitCoord = 0.5f * (Centroid.Min[SplitDim] + Centroid.Max[SplitDim]);

			// Partition the list of objects on this split
			// Fix bug: Originally 'i' starts from 'iStart + 1'.  
			// When 'm_pShapes[iStart]->GetCentroid()[SplitDim] < SplitCoord',
			// a wrong result would be obtained. (Although it make no 
			// difference to the final render result.) 
			for (uint32_t i = iStart; i < iEnd; i++)
			{
				if (m_pShapes[i]->GetCentroid()[SplitDim] < SplitCoord)
				{
					std::swap(m_pShapes[i], m_pShapes[iMid]);
					iMid++;
				}
			}

			// If we get a bad split, just choose the center...
			if (iMid == iStart || iMid == iEnd)
			{
				iMid = iStart + (iEnd - iStart) / 2;
			}
		}
		else if (m_SplitMethod == XML_ACCELERATION_BVH_SPLIT_METHOD_SAH)
		{
			constexpr uint32_t BUCKET_NUM = 12;
			BVHBucket Buckets[BUCKET_NUM];

			float InvNorm = 1.0f / (Centroid.Max[SplitDim] - Centroid.Min[SplitDim]);
			float InvSurfaceArea = 1.0f / BBox.GetSurfaceArea();

			// Divide the bounding box into several buckets
			for (uint32_t i = iStart; i < iEnd; i++)
			{
				uint32_t BucketIdx = uint32_t((BUCKET_NUM - 1) * (m_pShapes[i]->GetCentroid()[SplitDim] - Centroid.Min[SplitDim]) * InvNorm);
				CHECK(BucketIdx >= 0 && BucketIdx < BUCKET_NUM);

				Buckets[BucketIdx].nShape++;
				if (Buckets[BucketIdx].BBox.IsValid())
				{
					Buckets[BucketIdx].BBox.ExpandBy(m_pShapes[i]->GetBoundingBox());
				}
				else
				{
					Buckets[BucketIdx].BBox = m_pShapes[i]->GetBoundingBox();
				}
			}
			
			// Compute the cost
			float Cost[BUCKET_NUM - 1];
			for (uint32_t i = 0; i < BUCKET_NUM - 1; i++)
			{
				BoundingBox3f LeftBox, RightBox;
				uint32_t nLeftShapes = 0, nRightShapes = 0;

				// Left
				LeftBox = Buckets[0].BBox;
				for (uint32_t j = 0; j <= i; j++)
				{
					LeftBox.ExpandBy(Buckets[j].BBox);
					nLeftShapes += Buckets[j].nShape;
				}

				// Right
				RightBox = Buckets[i + 1].BBox;
				for (uint32_t j = i + 1; j <= BUCKET_NUM - 1; j++)
				{
					RightBox.ExpandBy(Buckets[j].BBox);
					nRightShapes += Buckets[j].nShape;
				}

				Cost[i] = 0.125f + (
					LeftBox.GetSurfaceArea() * nLeftShapes + 
					RightBox.GetSurfaceArea() * nRightShapes
					) * InvSurfaceArea;
			}

			// Find bucket whose cost is minimal
			float MinCost = Cost[0];
			uint32_t iMinCostSplitBucket = 0;
			for (uint32_t i = 0; i < BUCKET_NUM - 1; i++)
			{
				if (Cost[i] < MinCost)
				{
					MinCost = Cost[i];
					iMinCostSplitBucket = i;
				}
			}

			// Split nodes
			auto iIterMid = std::partition(
				m_pShapes.begin() + iStart,
				m_pShapes.begin() + iEnd,
				[=](const Shape * pShape)
				{
					uint32_t BucketIdx = uint32_t((BUCKET_NUM - 1) * (pShape->GetCentroid()[SplitDim] - Centroid.Min[SplitDim]) * InvNorm);
					CHECK(BucketIdx >= 0 && BucketIdx < BUCKET_NUM);
					return BucketIdx <= iMinCostSplitBucket;
				}
			);
			iMid = uint32_t(iIterMid - m_pShapes.begin());
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

	LOG(INFO) << "Build BVH (" << m_nNodes << " nodes, with " << m_nLeafs << " leafs) in " << 
		BVHBuildTimer.ElapsedString() << " and take " << MemString(m_nNodes * sizeof(BVHFlatNode)) << ".";
}

bool BVHAcceleration::RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const
{
	bool bFoundIntersection = false;       // Was an intersection found so far?
	Shape * pFoundShape = nullptr;

	const uint32_t STACK_MAX_SIZE = 1024;
	BVHTraversal Stack[STACK_MAX_SIZE];
	uint32_t iStackPtr = 0;

	Ray3f RayCopy(Ray); /// Make a copy of the ray (we will need to update its '.MaxT' value)

	// Push the root node
	Stack[iStackPtr].Idx = 0;
	Stack[iStackPtr].MinT = -std::numeric_limits<float>::max();
	iStackPtr++;

	while (iStackPtr > 0)
	{
		// Pop the next node
		BVHTraversal TopNode = Stack[--iStackPtr];
		uint32_t Idx = TopNode.Idx;
		float MinT = TopNode.MinT;

		BVHFlatNode CurrentFlatNode = m_pFlatTree[Idx];

		// If the node is further than the cloest found intersection, continue
		if (MinT > Isect.T)
		{
			continue;
		}

		// Leaf node -> Check intersection
		if (CurrentFlatNode.nRightChildOffset == 0)
		{
			for (uint32_t i = 0; i < CurrentFlatNode.nShapes; i++)
			{
				float U, V, T;
				Shape * pShape =  m_pShapes[CurrentFlatNode.iStart + i];
				if (pShape->RayIntersect(RayCopy, U, V, T))
				{
					if (bShadowRay)
					{
						return true;
					}

					RayCopy.MaxT = Isect.T = T;
					Isect.UV = Point2f(U, V);
					Isect.pShape = pShape;

					pFoundShape = pShape;
					bFoundIntersection = true;
				}
			}
		}
		// Not a leaf
		else
		{
			float HitLeftNearT, HitLeftFarT;
			float HitRightNearT, HitRightFarT;
			uint32_t iNearNode, iFarNode;
			float HitNearNodeNearT, HitFarNodeNearT;

			uint32_t iLeftNode = Idx + 1;
			uint32_t iRightNode = Idx + CurrentFlatNode.nRightChildOffset;

			const BVHFlatNode & LeftFlatNode = m_pFlatTree[iLeftNode];
			const BVHFlatNode & RightFlatNode = m_pFlatTree[iRightNode];
			
			bool bHitLeft = LeftFlatNode.BBox.RayIntersect(RayCopy, HitLeftNearT, HitLeftFarT);
			bool bHitRight = RightFlatNode.BBox.RayIntersect(RayCopy, HitRightNearT, HitRightFarT);

			if (bHitLeft && bHitRight)
			{
				if (HitRightNearT < HitLeftNearT)
				{
					iNearNode = iRightNode;
					iFarNode = iLeftNode;
					HitNearNodeNearT = HitRightNearT;
					HitFarNodeNearT = HitLeftNearT;
				}
				else
				{
					iNearNode = iLeftNode;
					iFarNode = iRightNode;
					HitNearNodeNearT = HitLeftNearT;
					HitFarNodeNearT = HitRightNearT;
				}

				// Push the farther first and then the near one
				Stack[iStackPtr++] = BVHTraversal(iFarNode, HitFarNodeNearT);
				Stack[iStackPtr++] = BVHTraversal(iNearNode, HitNearNodeNearT);
			}
			else if (bHitLeft)
			{
				Stack[iStackPtr++] = BVHTraversal(iLeftNode, HitLeftNearT);
			}
			else if (bHitRight)
			{
				Stack[iStackPtr++] = BVHTraversal(iRightNode, HitRightNearT);
			}
		}
	}

	if (bFoundIntersection)
	{
		pFoundShape->PostIntersect(Isect);
		Isect.ComputeScreenSpacePartial(Ray);
	}

	return bFoundIntersection;
}

std::string BVHAcceleration::ToString() const
{
	return tfm::format(
		"BVHAcceleration[\n"
		"  leafSize = %s,\n"
		"  node = %s,\n"
		"  leafNode = %f\n"
		"  splitMethod = %s\n"
		"]",
		m_LeafSize,
		m_nNodes,
		m_nLeafs,
		m_SplitMethod
	);
}

NAMESPACE_END
