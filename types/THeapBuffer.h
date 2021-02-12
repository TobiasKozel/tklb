#ifndef TKLB_HEAPBUFFER
#define TKLB_HEAPBUFFER

// TKLB_INCLUDE_STD_START
#include <cmath>
#include <cstring>
#include <memory>
// TKLB_INCLUDE_STD_END

// TKLB_INCLUDE_START
#include "../util/TNoCopy.h"
#include "../util/TAssert.h"
#include "../util/TPrint.h"
// TKLB_INCLUDE_END

namespace tklb {

/**
 * Basically a bad std::vector which can also work with
 * foreign memory
 */
template <typename T, class Allocator = std::allocator<T>>
class HeapBuffer {
	using uint = unsigned int;
	T* mBuf = nullptr;
	uint mSize = 0; // size of elements requested
	uint mRealSize = 0; // the actually allocated size
	uint mGranularity; // the space actually allocated will be a multiple of this
	bool mInjected = false; // True if the memory doesn't belong to this instance
	TKLB_ASSERT_STATE(bool INJECTED_IS_CONST) // Set if the injected memory is const
	Allocator allocator;

#if TKLB_HEAP_DEBUG_SIZE > 0
	/**
	 * This is only for convenient debugging
	 * Non const accessors will copy the heap buffer into this stack buffer
	 * So the first few elemnts can be viewed in the debugger
	 */
	T DEBUF_BUF[TKLB_HEAP_DEBUG_SIZE];
#endif

public:
	// We don't do implicit copying
	TKLB_NO_COPY(HeapBuffer)

	/**
	 * @brief Copy contructor copying the buffer and its properties
	 * Will also copy injected memory
	 */
	template <class NewAllocator = Allocator>
	HeapBuffer(const HeapBuffer<T, NewAllocator>& source) {
		setGranularity(source.mGranularity);
		set(source);
	}

	/**
	 * @param size Size in elements of the buffer
	 * @param data Memory to copy in
	 * @param granularity How big the real allocated chunks are
	 */
	HeapBuffer(const uint size = 0, const T* data = nullptr, const uint granularity = 1024 / sizeof(T)) {
		setGranularity(granularity);
		if (size != 0) {
			if (data != nullptr) { set(data, size); }
			else { resize(size); }
		}
	}

	~HeapBuffer() { resize(0); }

	/**
	 * @brief Provide foreign memory to borrow
	 * @param mem Modifyable memory to use
	 * @param size Size of the memory in elements
	 * @param realSize The actual size if it's chunk allocated
	 */
	void inject(T* mem, const uint size, const uint realSize = 0) {
		if (!mInjected && mBuf != nullptr) { resize(0); };
		TKLB_ASSERT_STATE(INJECTED_IS_CONST = false)
		mInjected = true;
		mBuf = mem;
		mSize = size;
		mRealSize = realSize == 0 ? size : realSize;
	}

	/**
	 * @brief Provide const foreign memory to use
	 * Using non const accessors on this HeapBuffer instance will cause assertions in debug mode
	 * Use const_cast and the non const inject function if this is not desired
	 * @param mem Non modifyable memory to use
	 * @param size Element T count
	 * @param realSize The actual size if it's chunk allocated
	 */
	void inject(const T* mem, const uint size, const uint realSize = 0) {
		inject(const_cast<T*>(mem), size, realSize);
		TKLB_ASSERT_STATE(INJECTED_IS_CONST = true)
	}

	void setGranularity(const uint granularity) {
		mGranularity = std::min(1u, granularity);
	}

	/**
	 * @brief Copy the memory in
	 * @param mem Memory to copy over
	 * @param size Element T count
	 */
	void set(const T* mem, const uint size) {
		resize(size);
		memcpy(mBuf, mem, sizeof(T) * size);
	}

	template <class NewAllocator = Allocator>
	void set(const HeapBuffer<T, NewAllocator>& src) {
		set(src.data(), src.size());
	}

	T* data() {
#if TKLB_HEAP_DEBUG_SIZE > 0
		memcpy(DEBUF_BUF, mBuf, sizeof(T) * std::min(100u, mSize));
#endif
		// Don't use non const access when using injected const memory
		TKLB_ASSERT(!INJECTED_IS_CONST)
		return mBuf;
	}

	const T* data() const { return mBuf; }

	const T& operator[](const uint index) const { return mBuf[index]; }

	T& operator[](const uint index) {
#if TKLB_HEAP_DEBUG_SIZE > 0
		memcpy(DEBUF_BUF, mBuf, sizeof(T) * std::min(100u, mSize));
#endif
		// Don't use non const access when using injected const memory
		TKLB_ASSERT(!INJECTED_IS_CONST)
		return mBuf[index];
	}

	/**
	 * @brief Resize the buffer
	 * If the memory is borrowed it will become unique and owned by this instance
	 * as soon as an allocation happens
	 * @param size The new size
	 * @param downsize Whether to downsize and reallocate
	 */
	T* resize(const uint size, const bool downsize = true) {
		const uint chunked = mGranularity * std::ceil(size / double(mGranularity));

		if (size == 0) {
			if (!mInjected && mRealSize > 0) { allocator.deallocate(mBuf, mRealSize); }
			mBuf = nullptr;
			mRealSize = 0;
		} else if (chunked != mRealSize) {
			T* temp = nullptr;
			if (chunked > mRealSize) { // Size up
				temp = allocator.allocate(chunked);
				if (mBuf != nullptr) {
					memcpy(temp, mBuf, mSize * sizeof(T));
					memset(temp + mSize, 0, (chunked - mSize) * sizeof(T));
				}
			} else if(downsize) { // size down
				temp = allocator.allocate(chunked);
				if (mBuf != nullptr) {
					memcpy(temp, mBuf, chunked * sizeof(T));
				}
			}
			if (temp != nullptr) { // an allocation occured
				if (!mInjected && mRealSize > 0) {
					allocator.deallocate(mBuf, mRealSize);
				}
				mInjected = false; // we own the memory now
				TKLB_ASSERT_STATE(INJECTED_IS_CONST = false)
				mBuf = temp;
			}
		}
		mRealSize = chunked;
		mSize = size;
		return mBuf;
	}

	/**
	 * Contructs objects
	 */
	void construct() {
		for (uint i = 0; i < size(); i++) {
			allocator.construct(mBuf + i);
		}
	}

	uint size() const { return mSize; }
};

} // namespace

#endif // TKLB_HEAPBUFFER
