#ifndef TKLBZ_HANDLE_BUFFER
#define TKLBZ_HANDLE_BUFFER

#include "./THeapBuffer.hpp"
namespace tklb {
	/**
	 * Handle to a object contained in a buffer
	 */
	using Handle = unsigned int;

	template <class T, typename MemberType = Handle>
	class HandleBuffer : public HeapBuffer<T, true> {

		using Base = HeapBuffer<T, true>;
		using Size = typename Base::Size;

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
			mOffset = Size(((char*) &(((T*)nullptr)->*member)) - ((char*)nullptr));
		}

		Size getLastFree() const { return mLastFree; }

		T* at(const Handle& handle) {
			const Handle index = (handle & MaskIndex) >> MaskSplit;
			if (index == mLastFree) { return nullptr; }	// element deleted
			if (Base::size() <= index) { return nullptr; }	// out of range
			T* element = Base::data() + index;
			const Handle refernceId = handle & MaskId;
			const Handle id = (*((Handle*)(((char*) element) + mOffset))) & MaskId;
			if (id != refernceId) { return nullptr; }	// Id mismatch, so object was replaced
			return element;
		}

		Handle create() {
			Size index = 0;
			const Size s = Base::size();
			if (s <= mLastFree || mLastFree < 0) {
				index = s;
				Base::resize(s + 1);
			}
			Size zero = 0; // should underflow
			mLastFree = zero - 1; // make sure free slot is not in size any more
			Handle ret = index << MaskSplit;
			Handle id = (*((Handle*)(((char*) (Base::data() + index)) + mOffset))) & MaskId;
			ret = ret | id;
			return ret;
		}

		/**
		 * @brief add element, may trigger resize
		 */
		Handle push(const T& object) {
			Handle ret = 0;
			if (Base::size() <= mLastFree || mLastFree < 0) {
				// no free slot
				ret = Base::size();
				if (!Base::push(object)) {
					return false; // something went wrong
				}
			} else {
				// use free slot
				ret = mLastFree;
				new (Base::data() + mLastFree) T(object);
			}
			ret = ret << MaskSplit;
			Handle id = (*((Handle*)(((char*) &object) + mOffset))) & MaskId;
			ret = ret | id;
			Size zero = 0; // should underflow
			mLastFree = zero - 1; // make sure free slot is not in size any more
			return ret;
		}

		bool pop(const Handle& handle, T* destination = nullptr) {
			T* element = at(handle);
			if (element == nullptr) { return false; }
			if (destination != nullptr) {
				memory::copy(destination, element, sizeof(T));
			}
			return remove(*element);
		}

		bool remove(const Size index) {
			if (Base::size() <= index) { return false; }
			if (index == mLastFree) { return false; }
			Base::data()[index].~T();
			mLastFree = index;
			return true;
		}

		bool remove(const T& object) {
			T* arr = Base::data();
			for (Size i = 0; i < Base::size(); i++) {
				if ((arr + i) == &object) {
					return remove(i);
				}
			}
			return false;
		}

	};
}

#endif // TKLBZ_HANDLE_BUFFER
