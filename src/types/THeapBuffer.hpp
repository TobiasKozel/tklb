#ifndef TKLBZ_HEAPBUFFER
#define TKLBZ_HEAPBUFFER

#include "../util/TAssert.h"
#include "../util/TMemory.hpp"

#include <cmath>


namespace tklb {

	/**
	 * Basically a bad std::vector which can also work with foreign memory.
	 * Classes stored inside need to have a default contructor
	 */
	template <typename T, bool Aligned = false>
	class HeapBuffer {
	public:
		using Size = unsigned int; // everything using the heapbuffer uses this type
		static constexpr Size DEFAULT_GRANULARITY = 64;

	private:
		T* mBuf = nullptr; // Underlying buffer
		Size mSize = 0; // size of elements requested
		Size mRealSize = 0; // the actually allocated size
		// the space actually allocated will be a multiple of this
		// if it's 0 the memory is not actually owned by the buffer
		Size mGranularity = DEFAULT_GRANULARITY; // can't be 0 at the start

		// True when the foreign memory is const, only cheked in debug mode
		TKLB_ASSERT_STATE(bool IS_CONST)

		/**
		 * @brief Allocated the exact size requsted and copies existing objects.
		 * Will not call their destructors or constructors!
		 */
		bool allocate(Size chunk) noexcept {
			T* oldBuf = mBuf;
			if (0 < chunk) {
				T* newBuf = nullptr;
				// TODO tklb Consider using realloc
				if (Aligned) {
					newBuf = reinterpret_cast<T*>(
						TKLB_MALLOC_ALIGNED(chunk * sizeof(T))
					);
				} else {
					newBuf = reinterpret_cast<T*>(
						TKLB_MALLOC(chunk * sizeof(T))
					);
				}
				if (newBuf == nullptr) {
					TKLB_ASSERT(false)
					return false; // ! Allocation failed
				}
				if (0 < mSize && oldBuf != nullptr && newBuf != nullptr) {
					// copy existing content
					memory::copy(newBuf, oldBuf, mSize * sizeof(T));
				}
				mBuf = newBuf;
			} else {
				mBuf = nullptr;
			}

			if (oldBuf != nullptr && !injected() && mRealSize > 0) {
				// Get rif of oldbuffer, object destructors were alredy called
				if (Aligned) {
					TKLB_FREE_ALIGNED(oldBuf);
				} else {
					TKLB_FREE(oldBuf);
				}
			}

			mGranularity = DEFAULT_GRANULARITY; // we definetly own the memory now
			TKLB_ASSERT_STATE(IS_CONST = false)
			mRealSize = chunk;
			return true;
		}

		Size closestChunkSize(Size chunk) const {
			TKLB_ASSERT(0 < mGranularity)
			return mGranularity * std::ceil(chunk / double(mGranularity));
		}

	public:

		/**
		 * @brief Setup the buffer with a size. User has to check if allocation was successful.
		 * @param size Size in elements of the buffer
		 * @param granularity How big the real allocated chunks are
		 */
		HeapBuffer(const Size size = 0, const Size granularity = DEFAULT_GRANULARITY) {
			setGranularity(granularity);
			if (size != 0) { resize(0); }
		}

		/**
		 * @brief Copy the contents of another buffer in
		 * See set()
		 * Failed allocations have to be checked
		 */
		HeapBuffer(const HeapBuffer<T>& source) {
			set(source);
		}

		HeapBuffer(const HeapBuffer*) = delete;
		HeapBuffer(HeapBuffer&&) = delete;
		HeapBuffer& operator= (const HeapBuffer&) = delete;
		HeapBuffer& operator= (HeapBuffer&&) = delete;

		~HeapBuffer() { resize(0); }

		/**
		 * @brief Resizes and copies the contents of the source Buffer
		 * This will do a memory::copy,so none of the object
		 * contructors will called
		 */
		bool set(const HeapBuffer<T>& source) {
			setGranularity(source.mGranularity);
			if (mBuf != nullptr) {
				resize(0); // Clear first so no old data gets copied on resize
			}
			if (!resize(source.size())) {
				return false; // ! Allocation failed
			}
			memory::copy(mBuf, source.data(), mSize * sizeof(T));
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
			mGranularity = 0;
			mBuf = mem;
			mSize = size;
			mRealSize = realSize == 0 ? size : realSize;
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

		void setGranularity(const Size granularity) {
			if (mGranularity == 0) {
				TKLB_ASSERT(false)
				return;
			}
			mGranularity = granularity;
		}

		T* data() {
			// Don't use non const access when using injected const memory
			TKLB_ASSERT(!IS_CONST)
			return mBuf;
		}

		const T* data() const { return mBuf; }

		const T& operator[](const Size index) const {
			return mBuf[index];
		}

		T& operator[](const Size index) {
			// Don't use non const access when using injected const memory
			TKLB_ASSERT(!IS_CONST)
			return mBuf[index];
		}

		/**
		 * @brief Will make sure the desired space is allocated
		 * @return Whether the allocation was succesful
		 */
		bool reserve(const Size size) {
			if (size  < mRealSize) { return true; }
			return allocate(closestChunkSize(size));
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
			const Size chunked = closestChunkSize(size);

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
		 */
		bool push(const T& object) {
			Size newSize = mSize + 1;
			if (mRealSize < newSize) {
				if (allocate(closestChunkSize(newSize))) {
					memory::copy(mBuf + mSize, &object, sizeof(T));
				} else {
					return false; // ! Allocation failed
				}
			} else {
				memory::copy(mBuf + mSize, &object, sizeof(T));
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
			memory::copy(object, mBuf + mSize - 1, sizeof(T));
			mSize--;
			return true;
		}

		/**
		 * @brief Removes the object at a given index and fills the gap with another
		 * Will never shrink the buffer/allocate
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

		bool remove(const T& object) {
			for (Size i = 0; i < mSize; i++) {
				if (mBuf[i] == object) {
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
		bool injected() const { return mGranularity == 0; }

		/**
		 * @brief Returns the amount of elements in the container
		 */
		Size size() const { return mSize; }

		/**
		 * @brief Returns the real allocated size in elements
		 */
		Size reserved() const { return mRealSize; }

	};

} // namespace

#endif // TKLB_HEAPBUFFER
