#ifndef _TKLB_HEAPBUFFER
#define _TKLB_HEAPBUFFER

#include <algorithm>	// For std::max
#include <cmath>		// For std::ceil
#include <cstring>


// #define TKLB_NO_STDLIB

#ifndef TKLB_NO_STDLIB
	#include "../memory/TAllocator.hpp"
#else
	namespace tklb {
		template <class T>
		struct DummyAllocator {
			T* allocate(size_t n) noexcept { return nullptr; }
			void deallocate(T* ptr, size_t n) noexcept { }
		};
	}
#endif // TKLB_NO_STDLIB


#ifndef TKLB_ASSERT
	#define TKLB_ASSERT(cond)
	#define TKLB_ASSERT_STATE(statement)
#endif

namespace tklb {

	/**
	 * @brief Basically a bad std::vector without exceptions which can also work with foreign memory.
	 * @tparam T Element type need to have a default contructor
	 * @tparam ALIGNMENT Memory alignment in bytes needs to be a power of 2. Defaults to 0
	 * @tparam ALLOCATOR Normal allocator class defaults to non basic malloc/free wrapper
	 * @tparam SIZE Size type used for index, defaults to unsigned int to save some space
	 */
	template <typename T,
		size_t ALIGNMENT = 0,
		class ALLOCATOR =
	#ifndef TKLB_NO_STDLIB
		StdAllocator
	#else
		DummyAllocator
	#endif
		<unsigned char>, // TODO this sucks, but need byte sized allocations for alignment
		typename SIZE = unsigned int
	>
	class HeapBuffer {
	public:
		/**
		 * everything using the heapbuffer uses this type
		 * Always needs to be unsigned!
		 */
		using Size = SIZE;

		static constexpr size_t Alignment = ALIGNMENT;

		static constexpr size_t ChunkSize = 16;

	private:
		ALLOCATOR mAllocator;
		T* mBuf = nullptr;			// Underlying buffer
		Size mSize = 0; // elements in buffer

		/**
		 * the actually allocated size
		 * if it's 0 the memory is not actually owned by the buffer
		 * and won't be delete with the object.
		 * No safety mechanism if memory becomes invalid
		 */
		Size mRealSize = 0;

		// True when the foreign memory is const, only cheked in debug mode
		TKLB_ASSERT_STATE(bool IS_CONST = false)

	public:
		/**
		 * @brief Setup the buffer with a size. User has to check if allocation was successful.
		 * @param size Size in elements of the buffer
		 */
		HeapBuffer(const Size size = 0) {
			if (size != 0) { resize(size); }
		}

		/**
		 * @brief Copy Constructor, calls set()
		 * Failed allocations have to be checked
		 */
		template <int Alignment2, class Allocator2, typename Size2>
		HeapBuffer(const HeapBuffer<T, Alignment2, Allocator2, Size2>& source) { set(source); }

		HeapBuffer(const T* data, Size size) { set(data, size); }

		/**
		 * @brief Move contructor, will mark the original buffer as injected
		 */
		HeapBuffer(HeapBuffer&& source) :
			mBuf(source.mBuf),
			mSize(source.mSize),
			mRealSize(source.mRealSize)
		{
			TKLB_ASSERT_STATE(IS_CONST = source.IS_CONST)
			source.disown();
		}

		HeapBuffer& operator= (HeapBuffer&& source) {
			TKLB_ASSERT_STATE(IS_CONST == source.IS_CONST)
			mBuf = source.mBuf;
			mSize = source.mSize;
			mRealSize = source.mRealSize;
			source.disown();
			return *this;
		}

		HeapBuffer& operator= (const HeapBuffer&) = delete;
		HeapBuffer(const HeapBuffer*) = delete;

		~HeapBuffer() { resize(0); }

		/**
		 * @brief Resizes and copies the contents of the source Buffer
		 * This will do a memory::copy,so none of the object
		 * contructors will called
		 * @return True on success
		 */
		 template <int Alignment2, class Allocator2, typename Size2>
		bool set(const HeapBuffer<T, Alignment2, Allocator2, Size2>& source) {
			return set(source.data(), source.size());
		}

		/**
		 * @brief Set data from array
		 * @param data data array to copy
		 * @param size size in elements of the data array
		 * @return True on success
		 */
		bool set(const T* data, Size size) {
			if (mBuf != nullptr) {
				resize(0); // Clear first so no old data gets copied on resize
			}
			if (!resize(size)) {
				return false; // ! Allocation failed
			}
			for (Size i = 0; i < mSize; i++) {
				new (mBuf + i) T(*(data + i));
			}
			return true;
		}

		/**
		 * @brief Provide foreign memory to borrow
		 * @param mem Modifyable memory to use
		 * @param size Size of the memory in elements
		 * @param realSize The actual size if it's chunk allocated
		 */
		void inject(T* mem, const Size size, const Size realSize = 0) {
			if (!injected() && mBuf != nullptr) { resize(0); };
			TKLB_ASSERT_STATE(IS_CONST = false)
			disown();
			mBuf = mem;
			mSize = size;
		}

		/**
		 * @brief Provide const foreign memory to use
		 * Using non const accessors will cause assertions in debug mode
		 * @param mem Non modifyable memory to use
		 * @param size Size of the memory in elements
		 * @param realSize The actual size if it's chunk allocated
		 */
		void inject(const T* mem, const Size size, const Size realSize = 0) {
			inject(const_cast<T*>(mem), size, realSize);
			TKLB_ASSERT_STATE(IS_CONST = true)
		}

		/**
		 * @brief The memory is no longer manager by this instance.
		 * Leak prone.
		 */
		void disown() {
			TKLB_ASSERT(mRealSize != 0)
			mRealSize = 0;
		}

		bool empty() const { return mSize == 0; }

		T* data() {
			// Don't use non const access when using injected const memory
			TKLB_ASSERT(!IS_CONST)
			return mBuf;
		}

		const T* data() const { return mBuf; }

		inline const T& operator[](const Size index) const {
			#ifdef TKLB_MEM_TRACE
				TKLB_ASSERT(index < mSize)
			#endif
			return mBuf[index];
		}

		inline T& operator[](const Size index) {
			#ifdef TKLB_MEM_TRACE
				TKLB_ASSERT(index < mSize)
			#endif
			// Don't use non const access when using injected const memory
			TKLB_ASSERT(!IS_CONST)
			return mBuf[index];
		}

		const T* begin() const { return mBuf; }
		const T* end () const { return mBuf + mSize; }

		T* begin() { return mBuf; }
		T* end () { return mBuf + mSize; }

		T& last() {
			TKLB_ASSERT(0 < mSize)
			return mBuf[mSize - 1];
		}

		T& first() {
			TKLB_ASSERT(0 < mSize)
			return mBuf[0];
		}

		/**
		 * @brief Will make sure the desired space is allocated
		 * @return Whether the allocation was succesful
		 */
		bool reserve(const Size size) {
			if (size  < mRealSize) { return true; }
			return allocate(size);
		}

		/**
		 * @brief Resize the buffer
		 * If the memory is borrowed it will become unique and owned by this instance
		 * as soon as an allocation happens
		 * @param size The new size
		 * @param downsize Whether to downsize and reallocate
		 * @return Whether the allocation was successful
		 */
		bool resize(const Size size, const bool downsize = true) {
			const Size chunked = closestChunkSize(size, ChunkSize);

			if (size < mSize && mBuf != nullptr) { // downsize means destroy objects
				for (Size i = size; i < mSize; i++) {
					mBuf[i].~T(); // Call destructor
				}
			}

			if (mRealSize < chunked || (downsize && (mRealSize > chunked)) ) {
				// Upsize or downsize + downsize requested
				if (!allocate(chunked)) {
					return false; // ! Allocation failed
				}
			}

			if (mSize < size) { // upsize means construct objects
				for (Size i = mSize; i < size; i++) {
					T* test = new (mBuf + i) T();
				}
			}

			mSize = size;
			return true;
		}

		/**
		 * @brief Push the object to the back of the buffer
		 * TODO tklb move version
		 */
		bool push(const T& object) {
			Size newSize = mSize + 1;
			if (mRealSize < newSize) {
				if (allocate(closestChunkSize(newSize, ChunkSize))) {
					new (mBuf + mSize) T(object);
				} else {
					TKLB_ASSERT(false)
					return false; // ! Allocation failed
				}
			} else {
				new (mBuf + mSize) T(object);
			}
			mSize = newSize;
			return true;
		}

		/**
		 * @brief Get the last element in the buffer.
		 * Will never shrink the buffer/allocate
		 */
		bool pop(T* object) {
			if (0 == mSize) { return false; }
			new (object) T(*(mBuf + mSize - 1));
			mSize--;
			return true;
		}

		/**
		 * @brief Removes the object at a given index and fills the gap with another
		 * Will never shrink the buffer/allocate.
		 */
		bool remove(const Size index) {
			if (mSize <= index) { return false; }
			mBuf[index].~T(); // Call destructor
			if (index != mSize - 1) { // fill the gap with the last element
				memcpy(mBuf + index, mBuf + (mSize - 1), sizeof(T));
			}
			mSize--;
			return true;
		}

		/**
		 * @brief If a == comparisin is possible the object itself can be used.
		 */
		bool remove(const T& object) {
			for (Size i = 0; i < mSize; i++) {
				if (mBuf[i] == object) {
					return remove(i);
				}
			}
			return false;
		}

		/**
		 * Otherwise use memory location
		 */
		bool remove(const T* object) {
			for (Size i = 0; i < mSize; i++) {
				if ((mBuf + i) == object) {
					return remove(i);
				}
			}
			return false;
		}

		/**
		 * @brief If T is a pointer type, delete will be called for
		 * all pointers in buffer and resize(0) is called
		 */
		bool destroyPointers() {
			if (!std::is_pointer<T>::value) {
				TKLB_ASSERT(false)
				return false;
			}
			for (Size i = 0; i < mSize; i++) {
				delete mBuf[i];
			}
			resize(0);
			return true;
		}

		/**
		 * @brief Clears the buffer but doesn't free the memory
		 */
		void clear() { resize(0, false); }

		/**
		 * @brief Whether the memory is managed by the instance or is borrowed.
		 */
		bool injected() const { return mRealSize == 0 && 0 < mSize && mBuf != nullptr; }

		/**
		 * @brief Returns the amount of elements in the container
		 */
		Size size() const { return mSize; }

		/**
		 * @brief Returns the real allocated size in elements
		 */
		Size reserved() const { return mRealSize; }

		/**
		 * @brief Returns the size of the allocated space.
		 */
		Size allocated() const { return mRealSize * sizeof(T); }

		static Size closestChunkSize(Size size, Size chunk) {
			const double chunkMin = std::max(double(chunk), 1.0);
			return chunk * Size(std::ceil(size / double(chunkMin)));
		}

		static bool isAligned(const void* ptr) {
			return size_t(ptr) % Alignment == 0;
		}

	private:
		/**
		 * @brief Allocated the exact size requsted and copies existing objects.
		 * Will not call their destructors or constructors!
		 */
		bool allocate(Size chunk) noexcept {
			T* oldBuf = mBuf;
			if (0 < chunk) {
				const Size bytes = chunk * sizeof(T);
				// TODO tklb Consider using realloc
				void* newBuf = mAllocator.allocate(bytes + Alignment);
				if (newBuf == nullptr) {
					TKLB_ASSERT(false);
					return false;
				}

				if (Alignment != 0) {
					// Mask with zeroes at the end to floor the pointer to an aligned block
					// for these casts to work, the type needs to be as wide as void*
					const size_t mask = ~(size_t(Alignment - 1));
					const size_t pointer = reinterpret_cast<size_t>(newBuf);
					const size_t floored = pointer & mask;
					const size_t aligned = floored + Alignment;

					// Not enough space before aligned memory to store original ptr
					// This only happens when malloc doesn't align to sizeof(size_t)
					TKLB_ASSERT(sizeof(size_t) <= (aligned - pointer))

					newBuf = reinterpret_cast<void*>(aligned);
					size_t* original = reinterpret_cast<size_t*>(newBuf) - 1;
					*(original) = pointer;
					TKLB_ASSERT(isAligned(newBuf))
				}


				if (0 < mSize && oldBuf != nullptr && newBuf != nullptr) {
					// copy existing content
					// TODO tklb only copy the new realsize which might be smaller
					memcpy(newBuf, oldBuf, std::min(mSize, chunk) * sizeof(T));
				}
				mBuf = (T*) newBuf;
			} else {
				mBuf = nullptr;
			}

			if (oldBuf != nullptr && !injected() && mRealSize > 0) {
				// Get rif of oldbuffer, object destructors were already called
				if (Alignment == 0) {
					mAllocator.deallocate(reinterpret_cast<unsigned char*>(oldBuf), mRealSize);
				} else {
					auto original = *(reinterpret_cast<void**>(oldBuf) - 1);
					mAllocator.deallocate(reinterpret_cast<unsigned char*>(original), mRealSize);
				}
			}
			TKLB_ASSERT_STATE(IS_CONST = false)
			mRealSize = chunk;
			// TKLB_CHECK_HEAP()
			return true;
		}
	};

} // namespace

#endif // _TKLB_HEAPBUFFER

