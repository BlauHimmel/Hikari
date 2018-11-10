#include <core\MemoryArena.hpp>

NAMESPACE_BEGIN

void * AllocAligned(size_t Size)
{
#if defined(PLATFORM_WINDOWS)
	return _aligned_malloc(Size, 64);
#else
	return memalign(64, Size);
#endif
}

void FreeAligned(void * pPtr)
{
	if (pPtr != nullptr)
	{
#if defined(PLATFORM_WINDOWS)
		_aligned_free(pPtr);
#else
		free(pPtr);
#endif
	}
}

MemoryArena::MemoryArena(size_t BlockSize) : m_nBlockSize(BlockSize) { }

MemoryArena::~MemoryArena()
{
	FreeAligned(m_pCurrentBlock);
	for (auto & Block : m_UsedBlocks)
	{
		FreeAligned(Block.second);
	}
	for (auto & Block : m_AvailableBlocks)
	{
		FreeAligned(Block.second);
	}
}

void * MemoryArena::Alloc(size_t nBytes)
{
	// Round up _nBytes_ to minimum machine alignment
	const int Align = alignof(std::max_align_t);

	static_assert((Align && !(Align & (Align - 1))), "Minimum alignment not a power of two");

	nBytes = (nBytes + Align - 1) & ~(Align - 1);

	if (m_iCurrentBlockPos + nBytes > m_iCurrentAllocSize)
	{
		// Add current block to _usedBlocks_ list
		if (m_pCurrentBlock)
		{
			m_UsedBlocks.push_back(std::make_pair(m_iCurrentAllocSize, m_pCurrentBlock));
			m_pCurrentBlock = nullptr;
			m_iCurrentAllocSize = 0;
		}

		// Get new block of memory for _MemoryArena_

		// Try to get memory block from _availableBlocks_
		for (auto Iter = m_AvailableBlocks.begin(); Iter != m_AvailableBlocks.end(); ++Iter)
		{
			if (Iter->first >= nBytes)
			{
				m_iCurrentAllocSize = Iter->first;
				m_pCurrentBlock = Iter->second;
				m_AvailableBlocks.erase(Iter);
				break;
			}
		}

		if (!m_pCurrentBlock)
		{
			m_iCurrentAllocSize = std::max(nBytes, m_nBlockSize);
			m_pCurrentBlock = AllocAligned<uint8_t>(m_iCurrentAllocSize);
		}

		m_iCurrentBlockPos = 0;
	}

	void * pRet = m_pCurrentBlock + m_iCurrentBlockPos;
	m_iCurrentBlockPos += nBytes;
	return pRet;
}

void MemoryArena::Reset()
{
	m_iCurrentBlockPos = 0;
	m_AvailableBlocks.splice(m_AvailableBlocks.begin(), m_UsedBlocks);
}

size_t MemoryArena::TotalAllocated() const
{
	size_t Total = m_iCurrentAllocSize;
	for (const auto & Alloc : m_UsedBlocks)
	{
		Total += Alloc.first;
	}
	for (const auto & Alloc : m_AvailableBlocks)
	{
		Total += Alloc.first;
	}
	return Total;
}

NAMESPACE_END

