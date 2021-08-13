#ifndef TKLBZ_HANDLE_BUFFER
#define TKLBZ_HANDLE_BUFFER

#include "./THeapBuffer.hpp"

namespace tklb {
	using Handle = unsigned int;

	template <class T, typename MemberType = Handle>
	class HandleBuffer : public HeapBuffer<T, true> {

		using Size = HeapBuffer<T, true>::Size;

		static constexpr int MaskSplit = 16; // at which bit to split the mask
		static constexpr Handle MaskId = (1 << (MaskSplit)) - 1;
		static constexpr Handle MaskIndex = ~MaskId;

		Size mLastFree = 0;
		/**
		 * Offset to the member variable which is used to identify an object. in bytes
		 * Should get optimized away since it depends on template arguments.
		 */
		Size mOffset;

	public:
		HandleBuffer(MemberType T::*member) {
			// nice
			mOffset = ((char*) &(((T*)nullptr)->*member)) - ((char*)nullptr);
		}

		T* at(const Handle& handle) {
			const Handle index = (handle & MaskIndex) >> MaskSplit;
			if (index == mLastFree) { return nullptr; }	// element deleted
			if (size() <= index) { return nullptr; }	// out of range
			T* element = data() + index;
			const Handle refernceId = handle & MaskId;
			const Handle id = (*((Handle*)(((char*) element) + mOffset))) & MaskId;
			if (id != refernceId) { return nullptr; }	// Id mismatch, so object was replaced
			return element;
		}

		/**
		 * @brief add element, may trigger resize
		 */
		Handle push(const T& object) {

			if (size() <= mLastFree || mLastFree < 0) {
				// no free slot
				if (!HeapBuffer::push(object)) {
					return false; // something went wrong
				}
			} else {
				// use free slot
				memory::copy(data() + mLastFree, &object, sizeof(T));
			}
			Size zero = 0; // should underflow
			mLastFree = zero - 1; // make sure free slot is not in size any more
			return true;
		}

		bool pop(T* object = nullptr) {
			// not used
			TKLB_ASSERT(false)
			return false;
		}

		bool remove(const Size index) {
			if (!HeapBuffer::remove(index)) {
				return false;
			}
			mLastFree = index;
			return true;
		}

		bool remove(const T& object) {
			for (Size i = 0; i < size(); i++) {
				if (mBuf[i] == object) {
					return remove(i);
				}
			}
			return false;
		}

	};
}

#endif // TKLBZ_HANDLE_BUFFER
