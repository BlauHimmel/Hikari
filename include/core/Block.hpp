#pragma once

/* =======================================================================
This file contains classes for parallel rendering of "image blocks".
* ======================================================================= */

#include <core\Common.hpp>
#include <core\Color.hpp>
#include <core\Vector.hpp>
#include <tbb\mutex.h>

/* Block size used for parallelization */
#define HIKARI_BLOCK_SIZE 32

NAMESPACE_BEGIN

/**
* \brief Weighted pixel storage for a rectangular subregion of an image
*
* This class implements storage for a rectangular subregion of a
* larger image that is being rendered. For each pixel, it records color
* values along with a weight that specifies the accumulated influence of
* nearby samples on the pixel (according to the used reconstruction filter).
*
* When rendering with filters, the samples in a rectangular
* region will generally also contribute to pixels just outside of
* this region. For that reason, this class also stores information about
* a small border region around the rectangle, whose size depends on the
* properties of the reconstruction filter.
*/
class ImageBlock : public Eigen::Array<Color4f, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
{
public:
	/**
	* Create a new image block of the specified maximum size
	* \param Size
	*     Desired maximum size of the block
	* \param pFilter
	*     Samples will be convolved with the image reconstruction
	*     filter provided here.
	*/
	ImageBlock(const Vector2i & Size, const ReconstructionFilter * pFilter);

	/// Configure the offset of the block within the main image
	void SetOffset(const Point2i & Offset);

	/// Return the offset of the block within the main image
	const Point2i & GetOffset() const;

	/// Configure the size of the block within the main image
	void SetSize(const Point2i & Size);

	/// Return the size of the block within the main image
	const Vector2i & GetSize() const;

	/// Return the border size in pixels
	int GetBorderSize() const;

	/**
	* \brief Turn the block into a proper bitmap
	*
	* This entails normalizing all pixels and discarding
	* the border region.
	*/
	std::unique_ptr<Bitmap> ToBitmap() const;

	/// Convert a bitmap into an image block
	void FromBitmap(const Bitmap & Bitmap);

	/// Clear all contents
	void Clear();

	/// Record a sample with the given position and radiance value
	void Put(const Point2f & Pos, const Color3f & Value);

	/**
	* \brief Merge another image block into this one
	*
	* During the merge operation, this function locks
	* the destination block using a mutex.
	*/
	void Put(ImageBlock & Block);

	/// Lock the image block (using an internal mutex)
	void Lock() const;

	/// Unlock the image block
	void Unlock() const;

	/// Return a human-readable string summary
	std::string ToString() const;

protected:
	Point2i m_Offset;
	Vector2i m_Size;
	int m_BorderSize = 0;
	std::unique_ptr<float[]> m_pFilter = nullptr;
	float m_FilterRadius = 0;
	std::unique_ptr<float[]> m_pWeightsX = nullptr;
	std::unique_ptr<float[]> m_pWeightsY = nullptr;
	float m_LookupFactor = 0;
	mutable tbb::mutex m_Mutex;
};

/**
* \brief Spiraling block generator
*
* This class can be used to chop up an image into many small
* rectangular blocks suitable for parallel rendering. The blocks
* are ordered in spiraling pattern so that the center is
* rendered first.
*/
class BlockGenerator
{
public:
	/**
	* \brief Create a block generator with
	* \param Size
	*      Size of the image that should be split into blocks
	* \param BlockSize
	*      Maximum size of the individual blocks
	*/
	BlockGenerator(const Vector2i & Size, int BlockSize);

	/**
	* \brief Return the next block to be rendered
	*
	* This function is thread-safe
	*
	* \return \c false if there were no more blocks
	*/
	bool Next(ImageBlock & Block);

	/// Return the total number of blocks
	int GetBlockCount() const;

protected:
	enum EDirection
	{
		ERight = 0, 
		EDown  = 1,
		ELeft  = 2,
		EUp    = 4
	};

	Point2i m_Block;
	Vector2i m_NumBlocks;
	Vector2i m_Size;
	int m_BlockSize;
	int m_NumSteps;
	int m_BlocksLeft;
	int m_StepsLeft;
	int m_Direction;
	tbb::mutex m_Mutex;
};

NAMESPACE_END