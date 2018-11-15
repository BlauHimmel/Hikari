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
		uint32_t iStart = 0;
		uint32_t nRightChildOffset = 0;
		uint32_t nShapes = 0;
	};

	BVHFlatNode * m_pFlatTree = nullptr;
	uint32_t m_LeafSize = 0;
	uint32_t m_nNodes = 0;
	uint32_t m_nLeafs = 0;
	std::string m_SplitMethod;
};

NAMESPACE_END