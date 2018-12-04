#pragma once

#include <core\Common.hpp>
#include <core\Ray.hpp>

NAMESPACE_BEGIN

/**
* \brief Generic n-dimensional bounding box data structure
*
* Maintains a minimum and maximum position along each dimension and provides
* various convenience functions for querying and modifying them.
*
* This class is parameterized by the underlying point data structure,
* which permits the use of different scalar types and dimensionalities, e.g.
* \code
* TBoundingBox<Vector3i> IntegerBBox(Point3i(0, 1, 3), Point3i(4, 5, 6));
* TBoundingBox<Vector2d> DoubleBBox(Point2d(0.0, 1.0), Point2d(4.0, 5.0));
* \endcode
*
* \tparam T The underlying point data type (e.g. \c Point2d)
* \ingroup libcore
*/
template <typename TPointType>
struct TBoundingBox
{
public:
	enum { Dimension = TPointType::Dimension };
	using PointType  = TPointType;
	using Scalar     = typename PointType::Scalar;
	using VectorType = typename PointType::VectorType;

	PointType Min; ///< Component-wise minimum 
	PointType Max; ///< Component-wise maximum 

	/**
	* \brief Create a new invalid bounding box
	*
	* Initializes the components of the minimum
	* and maximum position to \f$\infty\f$ and \f$-\infty\f$,
	* respectively.
	*/
	TBoundingBox()
	{
		Reset();
	}

	/// Create a collapsed bounding box from a single point
	TBoundingBox(const PointType & Pt) : Min(Pt), Max(Pt) { }

	/// Create a bounding box from two positions
	TBoundingBox(const PointType & Min, const PointType & Max) : Min(Min), Max(Max) { }

	/// Test for equality against another bounding box
	bool operator==(const TBoundingBox & BBox) const
	{
		return Min == BBox.Min && Max == BBox.Max;
	}

	/// Test for inequality against another bounding box
	bool operator!=(const TBoundingBox & BBox) const
	{
		return Min != BBox.Min || Max != BBox.Max;
	}

	/// Calculate the n-dimensional volume of the bounding box
	Scalar GetVolume() const
	{
		return (Max - Min).prod();
	}

	/// Calculate the n-1 dimensional volume of the boundary
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

	/// Return the center point
	PointType GetCenter() const
	{
		return (Max + Min) * Scalar(0.5f);
	}

	/**
	* \brief Check whether a point lies \a on or \a inside the bounding box
	*
	* \param Pt The point to be tested
	*
	* \param bStrict Set this parameter to \c true if the bounding
	*               box boundary should be excluded in the test
	*/
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

	/**
	* \brief Check whether a specified bounding box lies \a on or \a within
	* the current bounding box
	*
	* Note that by definition, an 'invalid' bounding box (where Min=\f$\infty\f$
	* and Max=\f$-\infty\f$) does not cover any space. Hence, this method will always
	* return \a true when given such an argument.
	*
	* \param bStrict Set this parameter to \c true if the bounding
	*               box boundary should be excluded in the test
	*/
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

	/**
	* \brief Check two axis-aligned bounding boxes for possible overlap.
	*
	* \param bStrict Set this parameter to \c true if the bounding
	*               box boundary should be excluded in the test
	*
	* \return \c true If overlap was detected.
	*/
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

	/**
	* \brief Calculate the smallest squared distance between
	* the axis-aligned bounding box and the point \c p.
	*/
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

	/**
	* \brief Calculate the smallest distance between
	* the axis-aligned bounding box and the point \c p.
	*/
	Scalar DistanceTo(const PointType & Pt) const
	{
		return std::sqrt(SquaredDistanceTo(Pt));
	}

	/**
	* \brief Calculate the smallest square distance between
	* the axis-aligned bounding box and \c bbox.
	*/
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

	/**
	* \brief Calculate the smallest distance between
	* the axis-aligned bounding box and \c bbox.
	*/
	Scalar DistanceTo(const TBoundingBox & BBox) const
	{
		return std::sqrt(SquaredDistanceTo(BBox));
	}

	/**
	* \brief Check whether this is a valid bounding box
	*
	* A bounding box \c bbox is valid when
	* \code
	* BBox.Min[Dim] <= BBox.Max[Dim]
	* \endcode
	* holds along each dimension \c Dim.
	*/
	bool IsValid() const
	{
		return (Max.array() >= Min.array()).all();
	}

	/// Check whether this bounding box has collapsed to a single point
	bool IsPoint() const
	{
		return (Max.array() == Min.array()).all();
	}

	/// Check whether this bounding box has any associated volume
	bool HasVolume() const
	{
		return (Max.array() > Min.array()).all();
	}

	/// Return the dimension index with the largest associated side length
	int GetMajorAxis() const
	{
		VectorType Dir = Max - Min;
		int Largest = 0;
		for (int i = 1; i < Dimension; ++i)
		{
			if (Dir[i] > Dir[Largest])
			{
				Largest = i;
			}
		}
		return Largest;
	}

	/// Return the dimension index with the shortest associated side length
	int GetMinorAxis() const
	{
		VectorType Dir = Max - Min;
		int Shortest = 0;
		for (int i = 1; i < Dimension; ++i)
		{
			if (Dir[i] < Dir[Shortest])
			{
				Shortest = i;
			}
		}
		return Shortest;
	}

	/**
	* \brief Calculate the bounding box extents
	* \return Max - Min
	*/
	VectorType GetExtents() const
	{
		return Max - Min;
	}

	/// Clip to another bounding box
	void Clip(const TBoundingBox & BBox)
	{
		Min = Min.cwiseMax(BBox.Min);
		Max = Max.cwiseMin(BBox.Max);
	}

	/**
	* \brief Mark the bounding box as invalid.
	*
	* This operation sets the components of the minimum
	* and maximum position to \f$\infty\f$ and \f$-\infty\f$,
	* respectively.
	*/
	void Reset()
	{
		Min.setConstant(std::numeric_limits<Scalar>::infinity());
		Max.setConstant(-std::numeric_limits<Scalar>::infinity());
	}

	/// Expand the bounding box to contain another point
	void ExpandBy(const PointType & Pt)
	{
		Min = Min.cwiseMin(Pt);
		Max = Max.cwiseMax(Pt);
	}

	/// Expand the bounding box to contain another bounding box
	void ExpandBy(const TBoundingBox & BBox)
	{
		Min = Min.cwiseMin(BBox.Min);
		Max = Max.cwiseMax(BBox.Max);
	}

	/// Merge two bounding boxes
	static TBoundingBox Merge(const TBoundingBox & BBox1, const TBoundingBox & BBox2)
	{
		return TBoundingBox(
			BBox1.Min.cwiseMin(BBox2.Min),
			BBox1.Max.cwiseMax(BBox2.Max)
		);
	}

	/// Return the index of the largest axis
	int GetLargestAxis() const
	{
		VectorType Extents = Max - Min;

		if (Extents[0] >= Extents[1] && Extents[0] >= Extents[2])
		{
			return 0;
		}
		else if (Extents[1] >= Extents[0] && Extents[1] >= Extents[2])
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	/// Return the position of a bounding box corner
	PointType GetCorner(int Index) const
	{
		PointType Result;
		for (int i = 0; i < Dimension; ++i)
		{
			Result[i] = (Index & (1 << i)) ? Max[i] : Min[i];
		}
		return Result;
	}

	/// Check if a ray intersects a bounding box
	bool RayIntersect(const Ray3f & Ray) const
	{
		float NearT = -std::numeric_limits<float>::infinity();
		float FarT = std::numeric_limits<float>::infinity();

		for (int i = 0; i < 3; i++)
		{
			float Origin = Ray.Origin[i];
			float MinVal = Min[i], MaxVal = Max[i];

			if (Ray.Direction[i] == 0)
			{
				if (Origin < MinVal || Origin > MaxVal)
				{
					return false;
				}
			}
			else
			{
				float T1 = (MinVal - Origin) * Ray.DirectionReciprocal[i];
				float T2 = (MaxVal - Origin) * Ray.DirectionReciprocal[i];

				if (T1 > T2)
				{
					std::swap(T1, T2);
				}

				NearT = std::max(T1, NearT);
				FarT = std::min(T2, FarT);

				if (!(NearT <= FarT))
				{
					return false;
				}
			}
		}

		return Ray.MinT <= FarT && NearT <= Ray.MaxT;
	}

	/// Check if a ray intersects a bounding box
	bool RayIntersect(const Ray3f & Ray, float & NearT, float & FarT) const
	{
		NearT = -std::numeric_limits<float>::infinity();
		FarT = std::numeric_limits<float>::infinity();

		for (int i = 0; i < 3; i++)
		{
			float Origin = Ray.Origin[i];
			float MinVal = Min[i], MaxVal = Max[i];

			if (Ray.Direction[i] == 0)
			{
				if (Origin < MinVal || Origin > MaxVal)
				{
					return false;
				}
			}
			else
			{
				float T1 = (MinVal - Origin) * Ray.DirectionReciprocal[i];
				float T2 = (MaxVal - Origin) * Ray.DirectionReciprocal[i];

				if (T1 > T2)
				{
					std::swap(T1, T2);
				}

				NearT = std::max(T1, NearT);
				FarT = std::min(T2, FarT);

				if (!(NearT <= FarT))
				{
					return false;
				}
			}
		}

		return true;
	}

	/// Return half the length of diagonal line of the bounding box
	float GetRadius() const
	{
		return (Max - Min).norm() * 0.5f;
	}

	/// Return a string representation of the bounding box
	std::string ToString() const
	{
		if (!IsValid())
		{
			return "BoundingBox[invalid]";
		}
		else
		{
			return tfm::format("BoundingBox[Min = %s, Max = %s]", Min.ToString(), Max.ToString());
		}
	}
};

NAMESPACE_END