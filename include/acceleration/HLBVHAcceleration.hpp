#pragma once

#include <core\Common.hpp>
#include <core\Acceleration.hpp>
#include <core\MemoryArena.hpp>
#include <atomic>

NAMESPACE_BEGIN

struct MortonShape;
struct BVHBuildNode;
struct LinearBVHNode;

class HLBVHAcceleration : public Acceleration
{
public:
	HLBVHAcceleration(const PropertyList & PropList);

	virtual ~HLBVHAcceleration();

	virtual void Build() override;

	virtual bool RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const;

	virtual std::string ToString() const override;

private:
	uint32_t LeftShift3(uint32_t X) const;
	uint32_t EncodeMorton3(const Vector3f & Vec) const;
	void RadixSort(std::vector<MortonShape> & MortonShapes) const;
	BVHBuildNode * EmitHLBVH(
		BVHBuildNode *& pBuildNodes,
		MortonShape * pMortonShapes,
		uint32_t nShape,
		uint32_t * nTotalNodes,
		uint32_t * nLeafNodes,
		std::vector<Shape*> & OrderedShapes,
		std::atomic<int> * nOrderedShapesOffset,
		int iFirstBitIdx
	);
	BVHBuildNode * BuildUpperSAH(
		std::vector<BVHBuildNode*> & TreeletRoots,
		uint32_t iStart,
		uint32_t iEnd
	);
	uint32_t FlattenBVHTree(BVHBuildNode * pNode, uint32_t * pOffset);

private:
	uint32_t m_LeafSize = 0;
	uint32_t m_nNodes = 0;
	uint32_t m_nLeafs = 0;
	MemoryArena m_MemoryArena;
	LinearBVHNode * m_pNodes;
};

NAMESPACE_END