#pragma once

#include <core\Common.hpp>
#include <core\Acceleration.hpp>

NAMESPACE_BEGIN

class BVHAcceleration : public Acceleration
{
public:
	BVHAcceleration(const PropertyList & PropList);

	virtual ~BVHAcceleration();

	virtual void Build() override;

	virtual bool RayIntersect(const Ray3f & Ray, Intersection & Isect, bool bShadowRay) const;

	virtual std::string ToString() const override;

private:
	struct BVHFlatNode
	{
		BoundingBox3f BBox;
		uint32_t iStart, nRightChildOffset, nPrimitives;
	};

	BVHFlatNode * m_pFlatTree;
	uint32_t m_LeafSize;
	uint32_t m_nNodes;
	uint32_t m_nLeafs;
};

NAMESPACE_END