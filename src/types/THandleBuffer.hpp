#ifndef _TKLB_HANDLE_BUFFER
#define _TKLB_HANDLE_BUFFER

#include "./THeapBuffer.hpp"
namespace tklb {

	/**
	 * @brief TODO broken don't use. A buffer to access contents via a Handle with validity checks
	 * @details Type stored in here needs to store it's own id in a Handle member.
	 * @tparam T type to store
	 * @tparam Handle type to index elements
	 * @tparam MaskSplit At what bit to split the Handle for validation
	 */
	template <class T, typename Handle = unsigned int, int MaskSplit = sizeof(Handle) * 4>
	class HandleBuffer : public HeapBuffer<T, true> {

		using Base = HeapBuffer<T, true>;
		using Size = typename Base::Size;

		static constexpr Handle MaskId = (1 << (MaskSplit)) - 1;
		static constexpr Handle MaskIndex = ~MaskId;

		Size mLastFree = 0;
		/**
		 * Offset to the member variable which is used to identify an object. in bytes
		 * Should get optimized away since it depends on template arguments.
		 */
		Size mOffset;

	public:

		/**
		 * @brief Since 0 is a valid handle, this is used to indicate a invalid one
		 */
		static constexpr Handle InvalidHandle = ~0;

		/**
		 * @brief Construct a new Handle Buffer object
		 * @param member Pointer to the member variable used for validation
		 */
		HandleBuffer(Handle T::*member) {
			// nice
			mOffset = Size(((char*) &(((T*)nullptr)->*member)) - ((char*)nullptr));
		}

		template <class Func>
		void forEach(const Func&& func) {
			for (Size i = 0; i < Base::size(); i++) {
				// if (*(Base::data() + i))
			}
		}

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
					return InvalidHandle; // something went wrong
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

#endif // _TKLB_HANDLE_BUFFER
