#include <core\Block.hpp>
#include <core\Bitmap.hpp>
#include <core\ReconstructionFilter.hpp>
#include <core\BoundingBox.hpp>
#include <tbb\tbb.h>

NAMESPACE_BEGIN

ImageBlock::ImageBlock(const Vector2i & Size, const ReconstructionFilter * pFilter) : 
	m_Offset(0, 0), m_Size(Size)
{
	if (pFilter != nullptr)
	{
		/* Tabulate the image reconstruction filter for performance reasons */
		m_FilterRadius = pFilter->GetRadius();
		m_BorderSize = int(std::ceil(m_FilterRadius - 0.5f));
		m_pFilter.reset(new float[HIKARI_FILTER_RESOLUTION + 1]);
		for (int i = 0; i < HIKARI_FILTER_RESOLUTION; i++)
		{
			float Pos = (m_FilterRadius * i) / HIKARI_FILTER_RESOLUTION;
			m_pFilter[i] = pFilter->Eval(Pos);
		}
		m_pFilter[HIKARI_FILTER_RESOLUTION] = 0.0f;
		m_LookupFactor = HIKARI_FILTER_RESOLUTION / m_FilterRadius;
		int WeightSize = int(std::ceil(2.0f * m_FilterRadius)) + 1;
		m_pWeightsX.reset(new float[WeightSize]);
		m_pWeightsY.reset(new float[WeightSize]);
		memset(m_pWeightsX.get(), 0, sizeof(float) * WeightSize);
		memset(m_pWeightsY.get(), 0, sizeof(float) * WeightSize);
	}

	/* Allocate space for pixels and border regions */
	resize(Size.y() + 2 * m_BorderSize, Size.x() + 2 * m_BorderSize);
}

void ImageBlock::SetOffset(const Point2i & Offset)
{
	m_Offset = Offset;
}

const Point2i & ImageBlock::GetOffset() const
{
	return m_Offset;
}

void ImageBlock::SetSize(const Point2i & Size)
{
	m_Size = Size;
}

const Vector2i & ImageBlock::GetSize() const
{
	return m_Size;
}

int ImageBlock::GetBorderSize() const
{
	return m_BorderSize;
}

std::unique_ptr<Bitmap> ImageBlock::ToBitmap() const
{
	std::unique_ptr<Bitmap> Result = std::make_unique<Bitmap>(m_Size);
	for (int y = 0; y < m_Size.y(); ++y)
	{
		for (int x = 0; x < m_Size.x(); ++x)
		{
			Result->coeffRef(y, x) = coeff(y + m_BorderSize, x + m_BorderSize).DivideByFilterWeight();
		}
	}
	return Result;
}

void ImageBlock::FromBitmap(const Bitmap & Bitmap)
{
	if (Bitmap.cols() != cols() || Bitmap.rows() != rows())
	{
		throw HikariException("Invalid bitmap dimensions!");
	}

	for (int y = 0; y < m_Size.y(); ++y)
	{
		for (int x = 0; x < m_Size.x(); ++x)
		{
			coeffRef(y, x) << Bitmap.coeff(y, x), 1.0f;
		}
	}
}

void ImageBlock::Clear()
{
	setConstant(Color4f());
}

void ImageBlock::Put(const Point2f & Pos, const Color3f & Value)
{
	if (!Value.IsValid())
	{
		LOG(ERROR) << "Integrator: computed an invalid radiance value: " << Value.ToString();
		return;
	}

	/* Convert to pixel coordinates within the image block */
	Point2f ConvertedPos(
		Pos.x() - 0.5f - (m_Offset.x() - m_BorderSize),
		Pos.y() - 0.5f - (m_Offset.y() - m_BorderSize)
	);

	/* Compute the rectangle of pixels that will need to be updated */
	BoundingBox2i BBox(
		Point2i(int(std::ceil(ConvertedPos.x() - m_FilterRadius)), int(std::ceil(ConvertedPos.y() - m_FilterRadius))),
		Point2i(int(std::floor(ConvertedPos.x() + m_FilterRadius)), int(std::floor(ConvertedPos.y() + m_FilterRadius)))
	);

	BBox.Clip(BoundingBox2i(Point2i(0, 0), Point2i(int(cols()) - 1, int(rows()) - 1)));

	/* Lookup values from the pre-rasterized filter */
	for (int x = BBox.Min.x(), idx = 0; x <= BBox.Max.x(); ++x)
	{
		m_pWeightsX[idx++] = m_pFilter[int(std::abs(x - ConvertedPos.x()) * m_LookupFactor)];
	}
	for (int y = BBox.Min.y(), idx = 0; y <= BBox.Max.y(); ++y)
	{
		m_pWeightsY[idx++] = m_pFilter[int(std::abs(y - ConvertedPos.y()) * m_LookupFactor)];
	}

	for (int y = BBox.Min.y(), yr = 0; y <= BBox.Max.y(); ++y, ++yr)
	{
		for (int x = BBox.Min.x(), xr = 0; x <= BBox.Max.x(); ++x, ++xr)
		{
			coeffRef(y, x) += Color4f(Value) * m_pWeightsX[xr] * m_pWeightsY[yr];
		}
	}
}

void ImageBlock::Put(ImageBlock & Block)
{
	Vector2i Offset = Block.GetOffset() - m_Offset + Vector2i::Constant(m_BorderSize - Block.GetBorderSize());
	Vector2i Size = Block.GetSize() + Vector2i(2 * Block.GetBorderSize());

	tbb::mutex::scoped_lock Lock(m_Mutex);

	block(Offset.y(), Offset.x(), Size.y(), Size.x()) += Block.topLeftCorner(Size.y(), Size.x());
}

void ImageBlock::Lock() const
{
	m_Mutex.lock();
}

void ImageBlock::Unlock() const
{
	m_Mutex.unlock();
}

std::string ImageBlock::ToString() const
{
	return tfm::format(
		"ImageBlock[Offset = %s, Size = %s]", 
		m_Offset.ToString(), m_Size.ToString()
	);
}

BlockGenerator::BlockGenerator(const Vector2i & Size, int BlockSize) : 
	m_Size(Size), m_BlockSize(BlockSize)
{
	m_NumBlocks = Vector2i(
		int(std::ceil(Size.x() / float(BlockSize))),
		int(std::ceil(Size.y() / float(BlockSize)))
	);
	m_BlocksLeft = m_NumBlocks.x() * m_NumBlocks.y();
	m_Direction = ERight;
	m_Block = Point2i(m_NumBlocks / 2);
	m_StepsLeft = 1;
	m_NumSteps = 1;
}

bool BlockGenerator::Next(ImageBlock & Block)
{
	tbb::mutex::scoped_lock Lock(m_Mutex);

	if (m_BlocksLeft == 0)
	{
		return false;
	}

	Point2i Pos = m_Block * m_BlockSize;
	Block.SetOffset(Pos);
	Block.SetSize((m_Size - Pos).cwiseMin(Vector2i::Constant(m_BlockSize)));

	if (--m_BlocksLeft == 0)
	{
		return true;
	}

	do
	{
		switch (m_Direction)
		{
		case ERight: ++m_Block.x(); break;
		case EDown:  ++m_Block.y(); break;
		case ELeft:  --m_Block.x(); break;
		case EUp:    --m_Block.y(); break;
		}

		if (--m_StepsLeft == 0)
		{
			m_Direction = (m_Direction + 1) % 4;
			if (m_Direction == ELeft || m_Direction == ERight)
			{
				++m_NumSteps;
			}
			m_StepsLeft = m_NumSteps;
		}
	} while ((m_Block.array() < 0).any() || (m_Block.array() >= m_NumBlocks.array()).any());

	return true;
}

int BlockGenerator::GetBlockCount() const
{
	return m_BlocksLeft;
}

NAMESPACE_END

