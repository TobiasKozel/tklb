#ifndef _TKLB_HEAPBUFFER
#define _TKLB_HEAPBUFFER

#include "./TTypes.hpp"
#include "../memory/TNew.hpp"
#include "../util/TMath.hpp"
#include "../util/TTraits.hpp"
#include "../memory/TAllocator.hpp"

#ifndef TKLB_ASSERT
	#define TKLB_ASSERT(cond)
#endif
#ifndef TKLB_ASSERT_STATE
	#define TKLB_ASSERT_STATE(statement)
#endif

namespace tklb {
	/**
	 * @brief Basically a bad std::vector without exceptions which can also work with borrowed memory.
	 * @tparam T Element type. Needs to have a default contructor
	 * @tparam ALIGNMENT Memory alignment in bytes needs to be a power of 2. Defaults to 0 for no alignment
	 * @tparam ALLOCATOR Normal allocator class defaults to non basic malloc/free wrapper
	 * @tparam SIZE Size type used for index, defaults to unsigned int to save some space
	 */
	template <
		typename T = unsigned char,
		SizeT ALIGNMENT = 0,
		class ALLOCATOR = DefaultAllocator<>,
		typename SIZE = unsigned int
	>
	class HeapBuffer {
	public:
		/**
		 * @brief everything using the heapbuffer uses this type to address elements.
		 */
		using Size = SIZE;
		static_assert(
			traits::IsUnsigned<Size>::value && !traits::IsFloat<Size>::value,
			"Size needs to be unsigned and an integer"
		);

		static constexpr Pointer Alignment = ALIGNMENT;
		static_assert(
			isPowerof2(Alignment) || Alignment == 0,
			"Alignment needs to be a power of 2 or 0 for the arithmetic to work."
		);

		/**
		 * @brief allocations will happens in these chunk sizes
		 *        TODO maybe expose this.
		 */
		static constexpr Size ChunkSize = 16;

	private:
		ALLOCATOR mAllocator;
		T* mBuf = nullptr;		// Underlying buffer
		Size mSize = 0;			// elements in buffer

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
		HeapBuffer() = default;
		/**
		 * @brief Setup the buffer with a size. User has to check if allocation was successful.
		 * @param size Size in elements of the buffer
		 */
		HeapBuffer(const Size size) {
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
			TKLB_ASSERT_STATE(IS_CONST = source.IS_CONST)
			mBuf = source.mBuf;
			mSize = source.mSize;
			mRealSize = source.mRealSize;
			source.disown();
			return *this;
		}

		HeapBuffer& operator= (const HeapBuffer&) = delete;
		HeapBuffer(const HeapBuffer*) = delete;

		~HeapBuffer() {
			if (injected()) { return; }
			resize(0);
		}

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
		 */
		void inject(T* mem, const Size size) {
			if (!injected() && mBuf != nullptr) { resize(0); };
			TKLB_ASSERT_STATE(IS_CONST = false)
			disown();
			TKLB_ASSERT(isAligned(mem))
			mBuf = mem;
			mSize = size;
		}

		/**
		 * @brief Provide const foreign memory to use
		 * Using non const accessors will cause assertions in debug mode
		 * @param mem Non modifyable memory to use
		 * @param size Size of the memory in elements
		 */
		void inject(const T* mem, const Size size) {
			inject(const_cast<T*>(mem), size);
			TKLB_ASSERT_STATE(IS_CONST = true)
		}

		/**
		 * @brief The memory is no longer manager by this instance.
		 * Leak prone.
		 */
		void disown() {
			if (empty()) { return; }
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
			#ifdef TKLB_MEMORY_CHECK
				TKLB_ASSERT(index < mSize)
			#endif
			return mBuf[index];
		}

		inline T& operator[](const Size index) {
			#ifdef TKLB_MEMORY_CHECK
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

			if (size < mSize && mBuf != nullptr && !injected()) { // downsize means destroy objects
				for (Size i = size; i < mSize; i++) {
					mBuf[i].~T();
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
					new (mBuf + i) T();
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
				memory::copy(mBuf + index, mBuf + (mSize - 1), sizeof(T));
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
			if (size == 0) { return 0; }
			if (size == chunk) {return size; }
			chunk = tklb::max(chunk, Size(1));
			if (size % chunk == 0) {
				return size;
			}
			// need an additioncal chunk to fit
			return ((size / chunk) + 1) * chunk;
		}

		static constexpr bool isAligned(const void* ptr) {
			return
				(ptr == nullptr) ?
				false :
				((Alignment == 0) ? true :
				(Pointer(ptr) & (Alignment - 1)) == 0);
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
					constexpr Pointer mask = ~(Pointer(Alignment - 1));
					const Pointer pointer = reinterpret_cast<Pointer>(newBuf);
					// This is the next smallest aligned address.
					const Pointer floored = pointer & mask;

					// the address is below the actual
					// allocated space and we need to jump to the next aligned address.
					const Pointer aligned = floored + Alignment;

					// Now there's a gap between the allocation and aligned address.
					// It needs to be large enough to store a pointer
					// malloc aligns to sizeof(void*), so this should always be the case.
					TKLB_ASSERT(sizeof(Pointer) <= (aligned - pointer))

					newBuf = reinterpret_cast<void*>(aligned);
					// Now store the ptr of the original allocation right before
					// the aligned one, so it can be accessed again for freeing it up.
					Pointer* original = reinterpret_cast<Pointer*>(newBuf) - 1;
					*(original) = pointer;
					TKLB_ASSERT(isAligned(newBuf))
				}


				if (0 < mSize && oldBuf != nullptr && newBuf != nullptr) {
					// copy existing content
					// TODO tklb only copy the new realsize which might be smaller
					memory::copy(newBuf, oldBuf, min(mSize, chunk) * sizeof(T));
				}
				mBuf = (T*) newBuf;
			} else {
				mBuf = nullptr;
			}

			if (oldBuf != nullptr && !injected() && mRealSize > 0) {
				// Get rid of oldbuffer, object destructors were already called
				if (Alignment == 0) {
					mAllocator.deallocate(reinterpret_cast<unsigned char*>(oldBuf), mRealSize);
				} else {
					// Restore the original unaligned address and use it to free the memory
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

