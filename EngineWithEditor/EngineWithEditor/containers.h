#pragma once

#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

using byte = char;
constexpr size_t KILOBYTE = 1024;
constexpr size_t MEGABYTE = KILOBYTE * 1024;
constexpr size_t GIGABYTE = MEGABYTE * 1024;

namespace hw
{
	class Memory
	{
	private:
		struct Block
		{
			bool inUse = false;
			byte* start = nullptr;
			size_t size = 0;

			Block() = default;
			Block(void* start, size_t size) : inUse(false), start((byte*)start), size(size) {}
			void Invalidate() { start = nullptr; size = 0; }
			bool IsValid() const { return !!start; }
		};

		static constexpr size_t _ControllerCapacity = MEGABYTE;
		static constexpr size_t _MemoryCapacity = GIGABYTE + MEGABYTE;

		size_t validBlocks = 0;
		Block* controller = new Block[_ControllerCapacity];
		byte* memory = new byte[_MemoryCapacity];

		inline size_t BlockMemoryIndex(size_t blockIndex) const noexcept
		{
			return controller[blockIndex].start - memory;
		}
		inline size_t RemainingSpace(size_t blockIndex) const noexcept
		{
			return _MemoryCapacity - BlockMemoryIndex(blockIndex);
		}
		inline bool IsValidBlock(size_t blockIndex) const
		{
			return blockIndex < validBlocks;
		}
		inline size_t LastBlockIndex() const
		{
			return validBlocks - 1;
		}
		inline bool IsLastBlock(size_t blockIndex) const
		{
			return blockIndex == LastBlockIndex();
		}

		inline void ShiftBlocksForwardOne(size_t blockIndex) noexcept
		{
			_ASSERT_EXPR(blockIndex + 1 < _ControllerCapacity, L"Insufficient capacity for memory block shift");
			Block* block = controller + blockIndex;
			size_t srcSize = validBlocks - blockIndex;
			size_t destSize = _ControllerCapacity - blockIndex;
			if (memmove_s(block + 1, destSize, block, srcSize) == 0)
				++validBlocks;
		}

		inline void _SubdivideBlock(size_t blockIndex, size_t size) noexcept
		{
			Block& block = controller[blockIndex];
			Block& nextBlock = controller[blockIndex + 1];

			nextBlock.size = block.size - size;
			nextBlock.start = block.start + size;
			block.size = size;
		}

		// Splits one block into two
		// Doesn't work if there are no remaining spaces in the controller
		void Fragment(size_t blockIndex, size_t size)
		{
			if (size == controller[blockIndex].size) return; // Don't need to subdivide; already enough
			if (size > controller[blockIndex].size) return _ASSERT_EXPR(false, L"Cannot fragment into larger size");
			if (blockIndex + 1 >= _ControllerCapacity) return _ASSERT_EXPR(false, L"Insufficient space to subdivide");

			ShiftBlocksForwardOne(blockIndex);
			_SubdivideBlock(blockIndex, size);
		}

		// Combines adjacent, out-of-use blocks into single blocks to better represent the free, contiguous memory
		// Call this when deallocating memory
		void Defrag()
		{
			size_t startIndex = 0;
			size_t endIndex;
			size_t contiguousSize;
			size_t newValidCount = validBlocks;
			bool anyChanges = false;
			while (IsValidBlock(startIndex))
			{
				for (startIndex; IsValidBlock(startIndex); ++startIndex)
				{
					if (!controller[startIndex].inUse) break;
				}
				if (controller[startIndex].inUse) break;
				contiguousSize = controller[startIndex].size;
				for (endIndex = startIndex + 1; IsValidBlock(endIndex); ++endIndex)
				{
					if (controller[endIndex].inUse) break;
					if (!anyChanges) [[unlikely]] anyChanges = true; // Will only do this one time during this function
					contiguousSize += controller[endIndex].size;
					controller[endIndex].Invalidate();
					newValidCount--;
				}
				controller[startIndex].size = contiguousSize;
				startIndex = endIndex;
			}
			if (!anyChanges) return;
			std::stable_partition(controller, controller + _ControllerCapacity, [](const Block& b) { return b.IsValid(); });
			validBlocks = newValidCount;
		}
		
		Block* FindFreeBlock(size_t size)
		{
			for (size_t i = 0; IsValidBlock(i); ++i)
			{
				if (!controller[i].inUse && controller[i].size >= size)
				{
					Fragment(i, size);
					return controller + i;
				}
			}
			return nullptr;
		}
		Block* FindBlockByStart(void* start)
		{
			for (size_t i = 0; IsValidBlock(i); ++i)
			{
				if (controller[i].start == start)
					return controller + i;
			}
			return nullptr;
		}

		Memory()
		{
			controller[0] = Block(memory, _MemoryCapacity);
			validBlocks = 1;
		}

	public:
		~Memory()
		{
			delete[] controller;
			delete[] memory;
		}
		static Memory& GetSingleton()
		{
			static Memory* m = new Memory();
			return *m;
		}

		__declspec(allocator) void* Allocate(const size_t _Count)
		{
			Block* block = FindFreeBlock(_Count);
			if (!block) throw std::bad_alloc();
			block->inUse = true;
			return block->start;
		}
		// It is safe to free freed memory
		void Deallocate(void* const _Ptr, const size_t _Count)
		{
			Block* block = FindBlockByStart(_Ptr);
			_ASSERT_EXPR(block->size == _Count, L"tried to deallocate potentially unowned block");
			block->inUse = false;
			Defrag();
		}
	};

	__declspec(allocator) void* Alloc(const size_t _Count)
	{
		return Memory::GetSingleton().Allocate(_Count);
	}
	void Dealloc(void* const _Ptr, const size_t _Count)
	{
		Memory::GetSingleton().Deallocate(_Ptr, _Count);
	}

	template <class _Ty>
	class Allocator
	{
	public:
		static_assert(!std::is_const_v<_Ty>, "Allocator<const T> is ill-formed.");

		using _From_primary = Allocator;

		using value_type = _Ty;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using propagate_on_container_move_assignment = std::true_type;

		constexpr Allocator() noexcept {}
		constexpr Allocator(const Allocator&) noexcept = default;
		template <class _Other>
		constexpr Allocator(const Allocator<_Other>&) noexcept {}
		constexpr ~Allocator() = default;
		constexpr Allocator& operator=(const Allocator&) = default;

		void deallocate(_Ty* const _Ptr, const size_t _Count)
		{
			Dealloc(_Ptr, _Count * sizeof(_Ty));
		}

		_Ty* allocate(const size_t _Count)
		{
			return static_cast<_Ty*>(Alloc(_Count * sizeof(_Ty)));
		}
	};

	template<class _Ty, size_t _Size>
	using array = std::array<_Ty, _Size>;
	template<class _Ty>
	using vector = std::vector<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using deque = std::deque<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using forward_list = std::forward_list<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using list = std::list<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using stack = std::stack<_Ty, hw::deque<_Ty>>;
	template<class _Ty>
	using queue = std::queue<_Ty, hw::deque<_Ty>>;
	template<class _Ty>
	using priority_queue = std::priority_queue<_Ty, hw::deque<_Ty>>;
	template<class _Ty>
	using set = std::set<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using multiset = std::multiset<_Ty, hw::Allocator<_Ty>>;
	template<class _Kty, class _Ty>
	using map = std::map<_Kty, _Ty, std::less<_Kty>, hw::Allocator<std::pair<const _Kty, _Ty>>>;
	template<class _Kty, class _Ty>
	using multimap = std::multimap<_Kty, _Ty, std::less<_Kty>, hw::Allocator<std::pair<const _Kty, _Ty>>>;
	template<class _Ty>
	using unordered_set = std::unordered_set<_Ty, hw::Allocator<_Ty>>;
	template<class _Ty>
	using unordered_multiset = std::unordered_multiset<_Ty, hw::Allocator<_Ty>>;
	template<class _Kty, class _Ty>
	using unordered_map = std::unordered_map<_Kty, _Ty, std::hash<_Kty>, std::equal_to<_Kty>, hw::Allocator<std::pair<const _Kty, _Ty>>>;
	template<class _Kty, class _Ty>
	using unordered_multimap = std::unordered_multimap<_Kty, _Ty, std::hash<_Kty>, std::equal_to<_Kty>, hw::Allocator<std::pair<const _Kty, _Ty>>>;
}
