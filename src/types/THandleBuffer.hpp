#ifndef _TKLB_HANDLE_BUFFER
#define _TKLB_HANDLE_BUFFER

#include "./THeapBuffer.hpp"

namespace tklb {
	/**
	 * @brief A handle buffer to store elements in.
	 *        Each space in the buffer has a generation counter which is increased each time a element
	 *        is removed from that spot, so the old handle becomes invalid and access is portected.
	 * @tparam T type to store
	 * @tparam Handle type to index elements
	 * @tparam GenerationBits At what bit to split the Handle for validation, defaults to 1/4 of the size.
	 *         Higher values mean more bits are used for the generation counter to ensure valid access.
	 */
	template <class T, typename Handle = unsigned int, int GenerationBits = 8, class ALLOCATOR = DefaultAllocator<>>
	class HandleBuffer {
		static constexpr Handle MaskBits = sizeof(Handle) * 8 - GenerationBits;
		static constexpr Handle MaskIndex		= (1 << MaskBits) - 1;
		static constexpr Handle MaskGeneration	= ~MaskIndex;

		struct Element {
			Handle next;				///< Linked list next element and the current generation number
			char value[sizeof(T)];  	///< Space for the element
		};

		HeapBuffer<Element, 0, ALLOCATOR> mElements;
		Handle mStart = 0;				///< Points to the first free element
		Handle mFree = 0;				///< Number of free slots

	public:
		HandleBuffer(Handle size) { resize(size); }
		HandleBuffer() { }

		static constexpr Handle InvalidHandle = ~0;
		Handle size() const { return mElements.size(); }
		Handle free() const { return mFree; }

		/**
		 * @brief Resize buffer, does not maintain data or handles for now !
		 */
		void resize(Handle size) {
			TKLB_ASSERT(size < MaskIndex) // Needs to be smaller than MaskIndex!

			mElements.resize(size);

			for (Handle i = 0; i < size; i++) {
				mElements[i].next = i + 1;
			}
			mElements.last().next = MaskIndex; // last has no next
			mFree = size;
		}

		T* at(const Handle& handle) const {
			if (handle == InvalidHandle) { return nullptr; }

			const Handle index = handle & MaskIndex;

			TKLB_ASSERT(index != MaskIndex) // can't index this, see constructor
			if (mElements.size() <  index) { return nullptr; }

			const Element& element = mElements[index];
			if ((element.next ^ handle) & MaskGeneration) { return nullptr; } // referencing removed element

			return (T*) &(element.value);
		}

		Handle create() {
			// No more space
			if (mStart == MaskIndex) { return InvalidHandle; }
			TKLB_ASSERT(mFree != 0)
			const Handle index = mStart;
			auto& element = mElements[index];
			new (&element.value) T();

			// remove element from list
			mStart = element.next & MaskIndex;
			mFree--;

			return index | (element.next & MaskGeneration);
		}

		bool remove(const Handle& handle) {
			if (handle == InvalidHandle) { return false; }

			const Handle index = handle & MaskIndex;

			TKLB_ASSERT(index != MaskIndex) // can't index this, see constructor
			if (mElements.size() <  index) { return false; } // just an invalid handle no assert

			Element& element = mElements[index];

			// Invalidate the handle by incremening the generation
			Handle generation = ((element.next & MaskGeneration) >> MaskBits);
			generation += 1;
			generation = (generation << MaskBits);

			// insert the free element back into the list
			element.next = generation | index;
			mStart = index;

			mFree++;
			return true;
		}

		template <class Func>
		void const iterate(Func&& func) {
			// TODO
			Handle itemsLeft = mElements.size() - mFree;
			for (Handle i = 0; i < mElements.size(); i++) {
				const auto& element = mElements[i];
				if (true) {
					func(*((T*) mElements[i].value), i | (element.next & MaskGeneration));
				}
			}
		}
	};
}

#endif // _TKLB_HANDLE_BUFFER
