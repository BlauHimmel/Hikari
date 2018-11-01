#pragma once

#include <core/Common.hpp>

NAMESPACE_BEGIN

template <typename TPointType>
struct TBoundingBox
{
	enum { Dimension = _PointType::Dimension };
	using PointType  = TPointType;
	using Scalar     = PointType::Scalar;
	using VectorType = PointType::VectorType;

	PointType Min;
	PointType Max;

	TBoundingBox()
	{
		Reset();
	}

	TBoundingBox(const PointType & Pt) : Min(Pt), Max(Pt) { }

	TBoundingBox(const PointType & Min, const PointType & Max) : Min(Min), Max(Max) { }

	bool operator==(const TBoundingBox & BBox) const
	{
		return Min == BBox.Min && Max == BBox.Max;
	}

	bool operator!=(const TBoundingBox & BBox) const
	{
		return Min != BBox.Min || Max != BBox.Max;
	}

	Scalar GetVolume() const
	{
		return (Max - Min).prod();
	}

	Scalar GetSurfaceArea() const
	{
		VectorType Dir = Max - Min;
		Scalar Result = Scalar(0);
		for (int i = 0; i < Dimension; ++i)
		{
			Scalar Term = Scalar(1);
			for (int j = 0; j < Dimension; ++j)
			{
				if (i == j)
				{
					continue;
				}
				Term *= Dir[j];
			}
			Result += Term;
		}
		return Scalar(2) * Result;
	}

	PointType GetCenter() const 
	{
		return (Max + Min) * Scalar(0.5f);
	}

	bool Contains(const PointType & Pt, bool bStrict = false) const
	{
		if (bStrict)
		{
			return (Pt.array() > Min.array()).all()
				&& (Pt.array() < Max.array()).all();
		}
		else
		{
			return (Pt.array() >= Min.array()).all()
				&& (Pt.array() <= Max.array()).all();
		}
	}

	bool Contains(const TBoundingBox & BBox, bool bStrict = false) const
	{
		if (bStrict)
		{
			return (BBox.Min.array() > Min.array()).all()
				&& (BBox.Max.array() < Max.array()).all();
		}
		else
		{
			return (BBox.Min.array() >= Min.array()).all()
				&& (BBox.Max.array() <= Max.array()).all();
		}
	}

	bool Overlaps(const TBoundingBox & BBox, bool bStrict = false) const
	{
		if (bStrict)
		{
			return (BBox.Min.array() < Max.array()).all()
				&& (BBox.Max.array() > Min.array()).all();
		}
		else
		{
			return (BBox.Min.array() <= Max.array()).all()
				&& (BBox.Max.array() >= Min.array()).all();
		}
	}

	Scalar SquaredDistanceTo(const PointType & Pt) const
	{
		Scalar Result = Scalar(0);
		for (int i = 0; i < Dimension; ++i)
		{
			Scalar Value = Scalar(0);
			if (Pt[i] < Min[i])
			{
				Value = Min[i] - Pt[i];
			}
			else if (Pt[i] > Max[i])
			{
				Value = Pt[i] - Max[i];
			}
			Result += Value * Value;
		}
		return Result;
	}

	Scalar DistanceTo(const PointType & Pt) const
	{
		return std::sqrt(SquaredDistanceTo(Pt));
	}

	Scalar SquaredDistanceTo(const TBoundingBox & BBox) const
	{
		Scalar Result = 0;
		for (int i = 0; i < Dimension; ++i)
		{
			Scalar Value = 0;
			if (BBox.Max[i] < Min[i])
			{
				Value = Min[i] - BBox.Max[i];
			}
			else if (BBox.Min[i] > Max[i])
			{
				Value = BBox.Min[i] - Max[i];
			}
			Result += Value * Value;
		}
		return Result;
	}

	Scalar DistanceTo(const TBoundingBox & BBox) const
	{
		return std::sqrt(SquaredDistanceTo(BBox));
	}

	bool IsValid() const
	{
		return (Max.array() >= Min.array()).all();
	}

	bool IsPoint() const
	{
		return (Max.array() == Min.array()).all();
	}

	bool HasVolume() const
	{
		return (Max.array() > Min.array()).all();
	}
};

NAMESPACE_END