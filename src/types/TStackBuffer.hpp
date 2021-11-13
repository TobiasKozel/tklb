#ifndef _TKLB_STACKBUFFER
#define _TKLB_STACKBUFFER

#include "../util/TAssert.h"
#include "../memory/TMemory.hpp"

#include <cmath>
#include <utility>

namespace tklb {
	/**
	 * Identical interface to HeapBuffer but preallocated on the stack
	 */
	template <typename T, size_t mRealSize>
	class StackBuffer {
	public:
		/**
		 * everything using the StackBuffer uses this type
		 * Always needs to be unsigned!
		 */
		using Size = unsigned int;

	private:
		char mBuf[mRealSize * sizeof(T)];	// Underlying buffer
		Size mSize = 0;						// elements in buffer

	public:
		/**
		 * @brief Setup the buffer with a size. User has to check if allocation was successful.
		 * @param size Size in elements of the buffer
		 */
		StackBuffer(const Size size = 0) {
			if (size != 0) { resize(size); }
		}

		/**
		 * @brief Copy Constructor, calls set()
		 * Failed allocations have to be checked
		 */
		template <size_t N>
		StackBuffer(const StackBuffer<T, N>& source) {
			set(source);
		}

		StackBuffer(const T* data, Size size) {
			set(data, size);
		}

		/**
		 * @brief Move contructor, will mark the original buffer as injected
		 * ! No move constructor for now
		 */
		// StackBuffer(StackBuffer&& source) :
		// 	mPool(source.mPool),
		// 	mBuf(source.mBuf),
		// 	mSize(source.mSize),
		// 	mRealSize(source.mRealSize)
		// {
		// 	TKLB_ASSERT_STATE(IS_CONST = source.IS_CONST)
		// 	source.mRealSize = 0;
		// }

		// No move assignment either
		// StackBuffer& operator= (StackBuffer&& source) {
		// 	mBuf = source.mBuf;
		// 	mSize = source.mSize;
		// 	mRealSize = source.mRealSize;
		// 	TKLB_ASSERT_STATE(IS_CONST = source.IS_CONST)
		// 	source.mRealSize = 0;
		// 	return *this;
		// }

		StackBuffer& operator= (const StackBuffer&) = delete;
		StackBuffer(const StackBuffer*) = delete;

		~StackBuffer() { resize(0); }

		/**
		 * @brief Resizes and copies the contents of the source Buffer
		 * This will do a memory::copy,so none of the object
		 * contructors will called
		 * @return True on success
		 */
		template <size_t N>
		bool set(const StackBuffer<T, N>& source) {
			return set(source.data(), source.size());
		}

		/**
		 * @brief Set data from array
		 * @param data data array to copy
		 * @param size size in elements of the data array
		 * @return True on success
		 */
		bool set(const T* data, Size size) {
			resize(0); // Clear first so no old data gets copied on resize
			resize(size);
			for (Size i = 0; i < mSize; i++) {
				new (reinterpret_cast<T*>(mBuf) + i) T(*(data + i));
			}
			return true;
		}

		/**
		 * ! Inject doesn't do anything
		 * @brief Provide foreign memory to borrow
		 * @param mem Modifyable memory to use
		 * @param size Size of the memory in elements
		 * @param realSize The actual size if it's chunk allocated
		 */
		void inject(T* mem, const Size size, const Size realSize = 0) {
			TKLB_ASSERT(false)
		}

		/**
		 * ! Inject doesn't do anything
		 * @brief Provide const foreign memory to use
		 * Using non const accessors will cause assertions in debug mode
		 * @param mem Non modifyable memory to use
		 * @param size Size of the memory in elements
		 * @param realSize The actual size if it's chunk allocated
		 */
		void inject(const T* mem, const Size size, const Size realSize = 0) {
			TKLB_ASSERT(false)
		}

		/**
		 * ! Doesn't do anything
		 * @brief The memory is no longer manager by this instance.
		 * Leak prone.
		 */
		void disown() {
			TKLB_ASSERT(false)
		}

		T* data() { return reinterpret_cast<T*>(mBuf); }

		const T* data() const { return reinterpret_cast<const T*>(mBuf); }

		const T& operator[](const Size index) const {
			#ifdef TKLB_MEM_TRACE
				TKLB_ASSERT(index < mSize)
			#endif
			return reinterpret_cast<const T*>(mBuf)[index];
		}

		T& operator[](const Size index) {
			#ifdef TKLB_MEM_TRACE
				TKLB_ASSERT(index < mSize)
			#endif
			return reinterpret_cast<T*>(mBuf)[index];
		}

		T& last() {
			TKLB_ASSERT(0 < mSize)
			return reinterpret_cast<T*>(mBuf)[mSize - 1];
		}

		T& first() {
			TKLB_ASSERT(0 < mSize)
			return reinterpret_cast<T*>(mBuf)[0];
		}

		/**
		 * ! Doesn't do anything
		 * @brief Will make sure the desired space is allocated
		 * @return Whether the allocation was succesful
		 */
		bool reserve(const Size size) {
			return size < mRealSize;
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
			TKLB_ASSERT(size < mRealSize)
			if (size < mSize) { // downsize means destroy objects
				for (Size i = size; i < mSize; i++) {
					reinterpret_cast<T*>(mBuf)[i].~T(); // Call destructor
				}
			}

			if (mSize < size) { // upsize means construct objects
				for (Size i = mSize; i < size; i++) {
					T* test = new (reinterpret_cast<T*>(mBuf) + i) T();
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
				// ! Can't resize stack buffer
				TKLB_ASSERT(false)
				return false; // ! Allocation failed
			} else {
				new (reinterpret_cast<T*>(mBuf) + mSize) T(object);
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
			new (object) T(*(reinterpret_cast<T*>(mBuf) + mSize - 1));
			mSize--;
			return true;
		}

		/**
		 * @brief Removes the object at a given index and fills the gap with another
		 * Will never shrink the buffer/allocate.
		 */
		bool remove(const Size index) {
			if (mSize <= index) { return false; }
			reinterpret_cast<T*>(mBuf)[index].~T(); // Call destructor
			if (index != mSize - 1) { // fill the gap with the last element
				memory::copy(
					reinterpret_cast<T*>(mBuf) + index,
					reinterpret_cast<T*>(mBuf) + (mSize - 1),
					sizeof(T)
				);
			}
			mSize--;
			return true;
		}

		/**
		 * @brief If a == comparisin is possible the object itself can be used.
		 */
		bool remove(const T& object) {
			for (Size i = 0; i < mSize; i++) {
				if (reinterpret_cast<T*>(mBuf)[i] == object) {
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
				// not using the allocator to delete since this is
				// only intended for pointer buffers
				delete reinterpret_cast<T*>(mBuf)[i];
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
		bool injected() const { return false; }

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
	};

} // namespace

#endif // _TKLB_STACKBUFFER
