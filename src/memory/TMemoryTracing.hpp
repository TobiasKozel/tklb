#ifndef _TKLB_MEMORY_TRACING
#define _TKLB_MEMORY_TRACING

namespace tklb { namespace memory { namespace tracer {
	static inline void* allocateTrace(size_t size, const char* file, int line) noexcept;
	static inline void* reallocateTrace(void* ptr, size_t size, const char* file, int line) noexcept;
	static inline void* clearallocateTrace(size_t num, size_t size, const char* file, int line) noexcept;
	static inline void deallocateTrace(void* ptr, const char* file, int line) noexcept;
	static inline void deallocateAlignedTrace(void* ptr, const char* file, int line) noexcept;
	static inline void* allocateAlignedTrace(const char* file, int line, size_t size, size_t align = DEFAULT_ALIGN) noexcept;

	template <class T, typename ... Args>
	static inline T* createTrace(const char* file, int line, Args&& ... args);

	template <class T>
	static inline void disposeTrace(T* ptr, const char* file, int line);

	static void checkHeap();
} } } // namespace memory::namespace::tracer

#define TKLB_MALLOC(size)				tklb::memory::tracer::allocateTrace(size, __FILE__, __LINE__)
#define TKLB_FREE(ptr)					tklb::memory::tracer::deallocateTrace(ptr, __FILE__, __LINE__)
#define TKLB_REALLOC(ptr, size) 		tklb::memory::tracer::reallocateTrace(ptr, size, __FILE__, __LINE__)
#define TKLB_CALLOC(num, size) 			tklb::memory::tracer::clearallocateTrace(num, size, __FILE__, __LINE__)
#define TKLB_MALLOC_ALIGNED(size, ...)	tklb::memory::tracer::allocateAlignedTrace(__FILE__, __LINE__, size, ##__VA_ARGS__)
#define TKLB_FREE_ALIGNED(ptr)			tklb::memory::tracer::deallocateAlignedTrace(ptr, __FILE__, __LINE__)
#define TKLB_NEW(T, ...)				tklb::memory::tracer::createTrace<T>(__FILE__, __LINE__, ##__VA_ARGS__)
#define TKLB_DELETE(ptr)				tklb::memory::tracer::disposeTrace(ptr, __FILE__, __LINE__)
#define TKLB_CHECK_HEAP()				tklb::memory::tracer::checkHeap();

#include "../util/TAssert.h"
#include "../types/THeapBuffer.hpp"

namespace tklb { namespace memory { namespace tracer {

	constexpr char TKLB_MAGIC_STRING[] = "tklbend";
	constexpr char TKLB_MAGIC_BACKUP_STRING[] = "tklback";

	struct MagicBlock;

	inline HeapBuffer<MagicBlock*> MagicBlocks;
	inline bool EXCLUDE_TRACE = false;

	/**
	 * @brief Struct inserted at the end of every allocation.
	 */
	struct MagicBlock {
		char magic[sizeof(TKLB_MAGIC_STRING)];
		char padding[1000]; // padding space used to protect the data after
		char magicBackup[sizeof(TKLB_MAGIC_BACKUP_STRING)];
		const char* file; // source file
		int line; // line in source file
		void* ptr; // the start of the allocated mem
		size_t size;
		MagicBlock(const char* f, int l, void* p, size_t s) {
			file = f;
			line = l;
			ptr = p;
			size = s;
			memory::set(magic, 0, sizeof(magic));
			memory::copy(magic, TKLB_MAGIC_STRING, sizeof(TKLB_MAGIC_STRING));
			memory::copy(magicBackup, TKLB_MAGIC_BACKUP_STRING, sizeof(TKLB_MAGIC_BACKUP_STRING));
		}

		/**
		 * @brief Placement new at the end of the allocation
		 */
		static void construct(void* ptr, size_t s, const char* f, int l) {
			if (ptr == nullptr) { return; }
			MagicBlock* loc = reinterpret_cast<MagicBlock*>(static_cast<char*>(ptr) + s);
			MagicBlock* block = new (loc) MagicBlock(f, l, ptr, s);
			EXCLUDE_TRACE = true;
			TKLB_ASSERT(block != nullptr)
			MagicBlocks.push(block);
			EXCLUDE_TRACE = false;
		}

		static bool compare(const char* a, const char* b, size_t s) {
			for (size_t i = 0; i < s; i++) {
				if (a[i] != b[i]) {
					return false;
				}
			}
			return true;
		}

		/**
		 * @brief checks if the allocation is still intact
		 */
		static void check(MagicBlock* block) {
			if (!compare(block->magic, TKLB_MAGIC_STRING, sizeof(TKLB_MAGIC_STRING))) {
				if (!compare(block->magicBackup, TKLB_MAGIC_BACKUP_STRING, sizeof(TKLB_MAGIC_BACKUP_STRING))) {
					// Magic backup string is wrong, this shouldn't happen and somethings wrong
					TKLB_ASSERT(false)
				}
				TKLB_ASSERT(false) // Magic string is wrong so it was overrun
			}
		}

		/**
		 * @brief Find the MagicBlock for a given allocation.
		 */
		static MagicBlock* find(void* start) {
			if (MagicBlocks.data() == nullptr) { return nullptr; }
			for (size_t i = 0; i < MagicBlocks.size(); i++) {
				MagicBlock* block = MagicBlocks[i];
				if (block->ptr == start) {
					return block;
				}
			}
			return nullptr;
		}

		static void destroy(void* start) {
			if (start == nullptr) { return; }
			if (MagicBlocks.data() == nullptr) { return; }
			MagicBlock* block = find(start);
			TKLB_ASSERT(block != nullptr)
			check(block);
			MagicBlocks.remove(block);
		}
	}; // struct MagicBlock

	static inline void* allocateTrace(size_t size, const char* file, int line) noexcept {
		if (EXCLUDE_TRACE) {
			return allocate(size);
		}
		void* mem = allocate(size + sizeof(MagicBlock));
		MagicBlock::construct(mem, size, file, line);
		return mem;
	};

	static inline void* reallocateTrace(void* ptr, size_t size, const char* file, int line) noexcept {
		if (EXCLUDE_TRACE) {
			return reallocate(ptr, size);
		}
		MagicBlock::destroy(ptr);
		void* mem = reallocate(ptr, size + sizeof(MagicBlock));
		MagicBlock::construct(mem, size, file, line);
		return mem;
	};

	static inline void* clearallocateTrace(size_t num, size_t size, const char* file, int line) noexcept {
		if (EXCLUDE_TRACE) {
			return clearallocate(num, size);
		}
		// how much space clearallocate will allocate
		size_t total = num * size;
		// the amount of elements needed
		size_t numNeeded = ((total + sizeof(MagicBlock)) / size) + 1;
		void* mem = clearallocate(numNeeded, size);
		MagicBlock::construct(mem, total, file, line);
		return mem;
	};

	static inline void deallocateTrace(void* ptr, const char* file, int line) noexcept {
		if (EXCLUDE_TRACE) {
			return deallocate(ptr);
		}
		if (ptr == nullptr) { return; }
		MagicBlock::destroy(ptr);
		deallocate(ptr);
	};

	static inline void deallocateAlignedTrace(void* ptr, const char* file, int line) noexcept {
		if (EXCLUDE_TRACE) {
			return deallocateAligned(ptr);
		}
		if (ptr == nullptr) { return; }
		MagicBlock::destroy(ptr);
		deallocateAligned(ptr);
	};

	static inline void* allocateAlignedTrace(const char* file, int line, size_t size, size_t align) noexcept {
		if (EXCLUDE_TRACE) {
			return allocateAligned(size, align);
		}
		void* mem = allocateAligned(size + sizeof(MagicBlock), align);
		MagicBlock::construct(mem, size, file, line);
		return mem;
	};

	template <class T, typename ... Args>
	static inline T* createTrace(const char* file, int line, Args&& ... args) {
		if (EXCLUDE_TRACE) {
			void* mem = allocate(sizeof(T));
			return new (mem) T(std::forward<Args>(args)...);
		}
		void* mem = allocate(sizeof(T) + sizeof(MagicBlock));
		if (mem == nullptr) { return nullptr; }
		MagicBlock::construct(mem, sizeof(T), file, line);
		T* obj = new (mem) T(std::forward<Args>(args)...);
		return obj;
	}

	template <class T>
	static inline void disposeTrace(T* ptr, const char* file, int line) {
		if (ptr == nullptr) { return; }
		if (!EXCLUDE_TRACE) {
			MagicBlock::destroy(ptr);
		}
		ptr->~T();
		deallocate(ptr);
	}

	/**
	 * @brief Checks the MagicBlocks for all allocations.
	 */
	static void checkHeap() {
		if (MagicBlocks.data() == nullptr) { return; }
		size_t total = 0;
		for (size_t i = 0; i < MagicBlocks.size(); i++) {
			MagicBlock* block = MagicBlocks[i];
			MagicBlock::check(block);
			total += block->size;
		}
		size_t blocks = MagicBlocks.allocated();
		size_t left = Allocated;
		left -= blocks;
		TKLB_ASSERT(total == left)
	}

	static void init() {
		EXCLUDE_TRACE = true;
		MagicBlocks.reserve(1024 * 1024);
		EXCLUDE_TRACE = false;
	}

	static void stop() {
		EXCLUDE_TRACE = true;
		MagicBlocks.resize(0);
	}
} } } // namespace tklb::memory::tracer

#endif // _TKLB_MEMORY_TRACING
