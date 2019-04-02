#include <acceleration\HLBVHAcceleration.hpp>
#include <core\Timer.hpp>
#include <core\Shape.hpp>
#include <tbb\tbb.h>

NAMESPACE_BEGIN

REGISTER_CLASS(HLBVHAcceleration, XML_ACCELERATION_HLBVH);

struct MortonShape
{
	uint32_t iShape = 0;
	uint32_t MortonCode = 0;
};

struct BVHBuildNode
{
	BoundingBox3f BBox;
	BVHBuildNode * pChildren[2] = { nullptr };
	uint32_t iSplitAxis = 0;
	uint32_t nFirstShapeOffset = 0;
	uint32_t nShape = 0;

	void InitLeaf(uint32_t iFirst, uint32_t N, const BoundingBox3f & B)
	{
		iSplitAxis = 3;
		nFirstShapeOffset = iFirst;
		nShape = N;
		BBox = B;
		pChildren[0] = pChildren[1] = nullptr;
	}

	void InitInterior(uint32_t iAxis, BVHBuildNode * pLeft, BVHBuildNode * pRight)
	{
		iSplitAxis = iAxis;
		pChildren[0] = pLeft;
		pChildren[1] = pRight;
		BBox = pLeft->BBox;
		BBox.ExpandBy(pRight->BBox);
		nShape = 0;
		nFirstShapeOffset = uint32_t(-1);
	}
};

struct HLBVHTreeLet
{
	uint32_t iStart;
	uint32_t nShape;
	BVHBuildNode * pNodes;
};

struct BVHBucket
{
	uint32_t nShape = 0;
	BoundingBox3f BBox;
};

struct LinearBVHNode
{
	BoundingBox3f BBox;
	union
	{
		uint32_t nShapeOffset;   // Leaf
		uint32_t nRightChildOffset;  // Interior
	};
	uint32_t nShape = 0;
	uint32_t iAxis = 0;
};

HLBVHAcceleration::HLBVHAcceleration(const PropertyList & PropList) :
	Acceleration(PropList)
{
	m_LeafSize = uint32_t(PropList.GetInteger(XML_ACCELERATION_HLBVH_LEAF_SIZE, DEFAULT_ACCELERATION_HLBVH_LEAF_SIZE));
}

HLBVHAcceleration::~HLBVHAcceleration()
{
	delete[] m_pNodes;
}

void HLBVHAcceleration::Build()
{
	Timer HLBVHBuildTimer;

	// Compute bounding box of all shapes centroids
	BoundingBox3f BBox = m_pShapes[0]->GetBoundingBox();
	for (uint32_t i = 1; i < m_pShapes.size(); i++)
	{
		BBox.ExpandBy(m_pShapes[i]->GetBoundingBox());
	}

	// Compute Morton indices of shapes
	Point3f BBoxExtents = BBox.GetExtents();
	std::vector<MortonShape> MortonShapes(m_pShapes.size());

	constexpr int MORTON_BITS = 10;
	constexpr int MORTON_SCALE = 1 << MORTON_BITS;

	tbb::blocked_range<int> MortonRange(0, int(m_pShapes.size()));
	auto MortonMap = [&](const tbb::blocked_range<int> & Range)
	{
		for (int i = Range.begin(); i < Range.end(); i++)
		{
			MortonShapes[i].iShape = i;
			MortonShapes[i].MortonCode = EncodeMorton3((m_pShapes[i]->GetCentroid() - BBox.Min).cwiseQuotient(BBoxExtents) * MORTON_SCALE);
		}
	};

	/// Uncomment the following line for single threaded computing
	//MortonMap(MortonRange);

	/// Default: parallel computing
	tbb::parallel_for(MortonRange, MortonMap);

	// Radix sort shape Morton indices
	RadixSort(MortonShapes);

	// Create LBVH treelets at bottom of BVH

	// Find intervals of shapes for each treelet
	std::vector<HLBVHTreeLet> TreeletsToBuild;
	for (uint32_t iStart = 0, iEnd = 1; iEnd <= MortonShapes.size(); iEnd++)
	{
		constexpr uint32_t MASK = 0b111111111111000000000000000000;

		if (iEnd == uint32_t(MortonShapes.size()) || ((MortonShapes[iStart].MortonCode & MASK) != (MortonShapes[iEnd].MortonCode & MASK)))
		{
			// Add entry to TreeletsToBuild for this treelet
			uint32_t nShape = iEnd - iStart;
			uint32_t nMaxBVHNodes = 2 * nShape;

			// For performance concerned, constructor should not be executed here
			BVHBuildNode * pNodes = m_MemoryArena.Alloc<BVHBuildNode>(nMaxBVHNodes, false);
			HLBVHTreeLet Treelet;
			Treelet.iStart = iStart;
			Treelet.nShape = nShape;
			Treelet.pNodes = pNodes;
			TreeletsToBuild.push_back(Treelet);

			iStart = iEnd;
		}
	}

	// Create HLBVH for treelets
	std::atomic<int> nAtomicTotal(0), nAtomicLeaf(0), nOrderedShapeOffset(0);
	std::vector<Shape*> OrderedShapes(m_pShapes.size());
	
	tbb::blocked_range<int> TreeletRange(0, int(TreeletsToBuild.size()));
	auto TreeletMap = [&](const tbb::blocked_range<int> & Range)
	{
		const int iFirstBitIdx = 29 - 12;
		for (int i = Range.begin(); i < Range.end(); i++)
		{
			uint32_t nTotalNodes = 0;
			uint32_t nLeafNodes = 0;
			HLBVHTreeLet & Treelet = TreeletsToBuild[i];

			Treelet.pNodes = EmitHLBVH(
				Treelet.pNodes,
				&MortonShapes[Treelet.iStart],
				Treelet.nShape,
				&nTotalNodes,
				&nLeafNodes,
				OrderedShapes,
				&nOrderedShapeOffset,
				iFirstBitIdx
			);

			nAtomicTotal += nTotalNodes;
			nAtomicLeaf += nLeafNodes;
		}
	};

	/// Uncomment the following line for single threaded building
	//TreeletMap(TreeletRange);

	/// Default: parallel building
	tbb::parallel_for(TreeletRange, TreeletMap);

	m_nNodes = nAtomicTotal;
	m_nLeafs = nAtomicLeaf;

	// Create and return SAH BVH from HLBVH treelets
	std::vector<BVHBuildNode*> FinishedTreelets;
	FinishedTreelets.reserve(TreeletsToBuild.size());
	for (HLBVHTreeLet & Treelet : TreeletsToBuild)
	{
		FinishedTreelets.push_back(Treelet.pNodes);
	}

	BVHBuildNode * pRoot = BuildUpperSAH(FinishedTreelets, 0, uint32_t(FinishedTreelets.size()));

	m_pShapes.swap(OrderedShapes);

	uint32_t nOffset = 0;
	m_pNodes = new LinearBVHNode[m_nNodes];
	FlattenBVHTree(pRoot, &nOffset);
	CHECK(m_nNodes == nOffset);

	m_MemoryArena.Release();

	LOG(INFO) << "Build HLBVH (" << m_nNodes << " nodes, with " << m_nLeafs << " leafs) in " << 
		HLBVHBuildTimer.ElapsedString() << " and take " <<  MemString(m_nNodes * sizeof(LinearBVHNode)) << ".";
}

bool HLBVHAcceleration::RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const
{
	if (m_pNodes == nullptr)
	{
		return false;
	}

	bool bFoundIntersection = false;       // Was an intersection found so far?
	Shape * pFoundShape = nullptr;
	
	Ray3f RayCopy(Ray);
	bool bDirNeg[3] = { RayCopy.DirectionReciprocal.x() < 0, RayCopy.DirectionReciprocal.y() < 0, RayCopy.DirectionReciprocal.z() < 0 };

	uint32_t nToVisitOffset = 0, iCurrentNodeIndex = 0;
	uint32_t iNodesToVisit[1024];

	while (true)
	{
		const LinearBVHNode * pLinearNode = &m_pNodes[iCurrentNodeIndex];

		if (pLinearNode->BBox.RayIntersect(RayCopy))
		{
			// Leaf node
			if (pLinearNode->nShape > 0)
			{
				for (uint32_t i = 0; i < pLinearNode->nShape; i++)
				{
					float U, V, T;
					Shape * pShape = m_pShapes[pLinearNode->nShapeOffset + i];
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

				if (nToVisitOffset == 0)
				{
					break;
				}

				iCurrentNodeIndex = iNodesToVisit[--nToVisitOffset];
			}
			// Interior node
			else
			{
				if (bDirNeg[pLinearNode->iAxis])
				{
					iNodesToVisit[nToVisitOffset++] = iCurrentNodeIndex + 1;
					iCurrentNodeIndex = pLinearNode->nRightChildOffset;
				}
				else
				{
					iNodesToVisit[nToVisitOffset++] = pLinearNode->nRightChildOffset;
					iCurrentNodeIndex = iCurrentNodeIndex + 1;
				}
			}
		}
		else
		{
			if (nToVisitOffset == 0)
			{
				break;
			}
			iCurrentNodeIndex = iNodesToVisit[--nToVisitOffset];
		}
	}

	if (bFoundIntersection)
	{
		pFoundShape->PostIntersect(Isect);
		Isect.ComputeScreenSpacePartial(Ray);
	}

	return bFoundIntersection;
}

std::string HLBVHAcceleration::ToString() const
{
	return tfm::format(
		"HLBVHAcceleration[\n"
		"  node = %s,\n"
		"  leafNode = %f\n"
		"]",
		m_nNodes,
		m_nLeafs
	);
}

uint32_t HLBVHAcceleration::LeftShift3(uint32_t X) const
{
	CHECK(X <= (1 << 10));
	if (X == (1 << 10))
	{
		X--;
	}

	// x = ---- --98 ---- ---- ---- ---- 7654 3210
	X = (X | (X << 16)) & 0b00000011000000000000000011111111;
	// x = ---- --98 ---- ---- 7654 ---- ---- 3210
	X = (X | (X << 8 )) & 0b00000011000000001111000000001111;
	// x = ---- --98 ---- 76-- --54 ---- 32-- --10
	X = (X | (X << 4 )) & 0b00000011000011000011000011000011;
	// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
	X = (X | (X << 2 )) & 0b00001001001001001001001001001001;

	return X;
}

uint32_t HLBVHAcceleration::EncodeMorton3(const Vector3f & Vec) const
{
	CHECK(Vec.x() >= 0.0f && Vec.y() >= 0.0f && Vec.z() >= 0.0f);
	return (
		(LeftShift3(uint32_t(Vec.z())) << 2) |
		(LeftShift3(uint32_t(Vec.y())) << 1) |
		 LeftShift3(uint32_t(Vec.x()))
		);
}

void HLBVHAcceleration::RadixSort(std::vector<MortonShape> & MortonShapes) const
{
	// TODO : Could be paralleled

	std::vector<MortonShape> TempVector(MortonShapes.size());
	constexpr int BIT_PER_PASS = 6;
	constexpr int BITS = 30;
	static_assert((BITS % BIT_PER_PASS) == 0, "Radix sort bitsPerPass must evenly divide nBits");

	constexpr int nPasses = BITS / BIT_PER_PASS;

	for (int Pass = 0; Pass < nPasses; ++Pass)
	{
		// Perform one pass of radix sort, sorting BIT_PER_PASS bits
		int LowBit = Pass * BIT_PER_PASS;

		// Set in and out vector pointers for radix sort pass
		std::vector<MortonShape> & In = (Pass & 1) ? TempVector : MortonShapes;
		std::vector<MortonShape> & Out = (Pass & 1) ? MortonShapes : TempVector;

		// Count number of zero bits in array for current radix sort bit
		constexpr int BUCKET_NUM = 1 << BIT_PER_PASS;
		int BucketCount[BUCKET_NUM] = { 0 };
		constexpr int BIT_MASK = (1 << BIT_PER_PASS) - 1;

		for (const MortonShape & MortonShape : In)
		{
			int BucketIdx = (MortonShape.MortonCode >> LowBit) & BIT_MASK;
			CHECK(BucketIdx >= 0 && BucketIdx < BUCKET_NUM);
			++BucketCount[BucketIdx];
		}

		// Compute starting index in output array for each bucket
		int OutIndex[BUCKET_NUM];
		OutIndex[0] = 0;
		for (int i = 1; i < BUCKET_NUM; ++i)
		{
			OutIndex[i] = OutIndex[i - 1] + BucketCount[i - 1];
		}

		// Store sorted values in output array
		for (const MortonShape & MortonShape : In)
		{
			int BucketIdx = (MortonShape.MortonCode >> LowBit) & BIT_MASK;
			Out[OutIndex[BucketIdx]++] = MortonShape;
		}
	}

	// Copy final result from TempVector, if needed
	if (nPasses & 1)
	{
		std::swap(MortonShapes, TempVector);
	}
}

BVHBuildNode * HLBVHAcceleration::EmitHLBVH(
	BVHBuildNode *& pBuildNodes,
	MortonShape * pMortonShapes,
	uint32_t nShape,
	uint32_t * nTotalNodes,
	uint32_t * nLeafNodes,
	std::vector<Shape*> & OrderedShapes,
	std::atomic<int> * nOrderedShapeOffset,
	int iFirstBitIdx
)
{
	CHECK(nShape > 0);

	// Create and return leaf node of HLBVH treelet
	if (iFirstBitIdx == -1 || nShape <= m_LeafSize)
	{
		(*nTotalNodes)++;
		(*nLeafNodes)++;

		BVHBuildNode * pNode = pBuildNodes++;

		// First fetch the nOrderedShapeOffset and then add nShape to it
		uint32_t nFirstShapeOffset = nOrderedShapeOffset->fetch_add(nShape);

		uint32_t ShapeIdx = pMortonShapes[0].iShape;

		BoundingBox3f BBox = m_pShapes[ShapeIdx]->GetBoundingBox();
		OrderedShapes[0 + nFirstShapeOffset] = m_pShapes[ShapeIdx];

		for (uint32_t i = 1; i < nShape; i++)
		{
			ShapeIdx = pMortonShapes[i].iShape;
			OrderedShapes[i + nFirstShapeOffset] = m_pShapes[ShapeIdx];
			BBox.ExpandBy(m_pShapes[ShapeIdx]->GetBoundingBox());
		}

		pNode->InitLeaf(nFirstShapeOffset, nShape, BBox);
		return pNode;
	}
	else
	{
		int Mask = 1 << iFirstBitIdx;

		// Advance to next subtree level if there's no HLBVH split for this bit
		if ((pMortonShapes[0].MortonCode & Mask) == (pMortonShapes[nShape - 1].MortonCode & Mask))
		{
			return EmitHLBVH(
				pBuildNodes,
				pMortonShapes,
				nShape,
				nTotalNodes,
				nLeafNodes,
				OrderedShapes,
				nOrderedShapeOffset,
				iFirstBitIdx - 1
			);
		}

		// Find HLBVH split point for this dimension
		uint32_t iSearchStart = 0, iSearchEnd = nShape - 1;
		while (iSearchStart + 1 != iSearchEnd)
		{
			CHECK(iSearchStart != iSearchEnd);
			uint32_t iMid = (iSearchStart + iSearchEnd) / 2;

			if ((pMortonShapes[iSearchStart].MortonCode & Mask) == (pMortonShapes[iMid].MortonCode & Mask))
			{
				iSearchStart = iMid;
			}
			else
			{
				CHECK((pMortonShapes[iMid].MortonCode & Mask) == (pMortonShapes[iSearchEnd].MortonCode & Mask));
				iSearchEnd = iMid;
			}
		}

		uint32_t nSplitOffset = iSearchEnd;
		CHECK(nSplitOffset <= nShape - 1);
		CHECK((pMortonShapes[nSplitOffset - 1].MortonCode & Mask) != (pMortonShapes[nSplitOffset].MortonCode & Mask));

		// Create and return interior HLBVH node
		(*nTotalNodes)++;

		BVHBuildNode * pNode = pBuildNodes++;
		BVHBuildNode * pLeft = EmitHLBVH(
			pBuildNodes,
			pMortonShapes,
			nSplitOffset,
			nTotalNodes,
			nLeafNodes,
			OrderedShapes,
			nOrderedShapeOffset,
			iFirstBitIdx - 1
		);
		BVHBuildNode * pRight = EmitHLBVH(
			pBuildNodes,
			&pMortonShapes[nSplitOffset],
			nShape - nSplitOffset,
			nTotalNodes,
			nLeafNodes,
			OrderedShapes,
			nOrderedShapeOffset,
			iFirstBitIdx - 1
		);
		uint32_t iAxis = uint32_t(iFirstBitIdx % 3);
		pNode->InitInterior(iAxis, pLeft, pRight);
		return pNode;
	}
}

BVHBuildNode * HLBVHAcceleration::BuildUpperSAH(
	std::vector<BVHBuildNode*> & TreeletRoots,
	uint32_t iStart, 
	uint32_t iEnd
)
{
	CHECK(iStart < iEnd);

	uint32_t nNodes = iEnd - iStart;
	if (nNodes == 1)
	{
		return TreeletRoots[iStart];
	}

	m_nNodes++;

	BVHBuildNode * pNode = m_MemoryArena.Alloc<BVHBuildNode>();

	// Compute bounds of all nodes under this HLBVH node
	BoundingBox3f BBox = TreeletRoots[iStart]->BBox;
	for (uint32_t i = iStart + 1; i < iEnd; i++)
	{
		BBox.ExpandBy(TreeletRoots[i]->BBox);
	}

	// Compute bound of HLBVH node centroids, choose split dimension
	BoundingBox3f Centroid(0.5f * (TreeletRoots[iStart]->BBox.Min + TreeletRoots[iStart]->BBox.Max));
	for (uint32_t i = iStart + 1; i < iEnd; i++)
	{
		Centroid.ExpandBy(0.5f * (TreeletRoots[i]->BBox.Min + TreeletRoots[i]->BBox.Max));
	}

	uint32_t iSplitDim = Centroid.GetMajorAxis();
	if (Centroid.Max[iSplitDim] == Centroid.Min[iSplitDim])
	{
		uint32_t iMid = (iStart + iEnd) / 2;
		CHECK(iMid > iStart && iMid < iEnd);

		pNode->InitInterior(
			iSplitDim,
			BuildUpperSAH(TreeletRoots, iStart, iMid),
			BuildUpperSAH(TreeletRoots, iMid, iEnd)
		);

		return pNode;
	}

	constexpr int BUCKET_NUM = 12;
	BVHBucket Buckets[BUCKET_NUM];

	float InvNorm = 1.0f / (Centroid.Max[iSplitDim] - Centroid.Min[iSplitDim]);
	float InvSurfaceArea = 1.0f / BBox.GetSurfaceArea();

	// Initialize Buckets for HLBVH SAH partition buckets
	for (uint32_t i = iStart; i < iEnd; i++)
	{
		uint32_t BucketIdx = uint32_t((BUCKET_NUM - 1) * (TreeletRoots[i]->BBox.GetCenter()[iSplitDim] - Centroid.Min[iSplitDim]) * InvNorm);
		CHECK(BucketIdx >= 0 && BucketIdx < BUCKET_NUM);

		Buckets[BucketIdx].nShape++;
		if (Buckets[BucketIdx].BBox.IsValid())
		{
			Buckets[BucketIdx].BBox.ExpandBy(TreeletRoots[i]->BBox);
		}
		else
		{
			Buckets[BucketIdx].BBox = TreeletRoots[i]->BBox;
		}
	}

	// Compute costs for splitting after each bucket
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

	// Find bucket to split at that minimizes SAH metric
	float MinCost = Cost[0];
	uint32_t iMinCostSplitBucket = 0;
	for (uint32_t i = 1; i < BUCKET_NUM - 1; i++)
	{
		if (Cost[i] < MinCost)
		{
			MinCost = Cost[i];
			iMinCostSplitBucket = i;
		}
	}

	auto MidIter = std::partition(
		&TreeletRoots[iStart],
		&TreeletRoots[iEnd - 1] + 1,
		[=](const BVHBuildNode * pNode)
		{
			uint32_t BucketIdx = uint32_t((BUCKET_NUM - 1) * (pNode->BBox.GetCenter()[iSplitDim] - Centroid.Min[iSplitDim]) * InvNorm);
			CHECK(BucketIdx >= 0 && BucketIdx < uint32_t(BUCKET_NUM));
			return BucketIdx <= iMinCostSplitBucket;
		}
	);

	uint32_t iMid = uint32_t(MidIter - &TreeletRoots[0]);
	CHECK(iMid > iStart && iMid < iEnd);

	pNode->InitInterior(
		iSplitDim,
		BuildUpperSAH(TreeletRoots, iStart, iMid),
		BuildUpperSAH(TreeletRoots, iMid, iEnd)
	);

	return pNode;
}

uint32_t HLBVHAcceleration::FlattenBVHTree(BVHBuildNode * pNode, uint32_t * pOffset)
{
	CHECK(*pOffset < m_nNodes);

	LinearBVHNode * pLinearNode = &m_pNodes[*pOffset];
	pLinearNode->BBox = pNode->BBox;

	uint32_t Offset = (*pOffset)++;

	// Leaf node
	if (pNode->nShape > 0)
	{
		CHECK(pNode->pChildren[0] == nullptr && pNode->pChildren[1] == nullptr);
		CHECK(pNode->nShape < 65536);

		pLinearNode->nShapeOffset = pNode->nFirstShapeOffset;
		pLinearNode->nShape = pNode->nShape;
	}
	// Interior node
	else
	{
		pLinearNode->iAxis = pNode->iSplitAxis;
		pLinearNode->nShape = 0;
		FlattenBVHTree(pNode->pChildren[0], pOffset);
		pLinearNode->nRightChildOffset = FlattenBVHTree(pNode->pChildren[1], pOffset);

		CHECK(pNode->BBox.IsValid() && pNode->pChildren[0]->BBox.IsValid() && pNode->pChildren[1]->BBox.IsValid());
		CHECK(pNode->BBox.Contains(pNode->pChildren[0]->BBox) && pNode->BBox.Contains(pNode->pChildren[1]->BBox));
	}

	return Offset;
}

NAMESPACE_END