#ifndef _TKLB_HANDLE_BUFFER
#define _TKLB_HANDLE_BUFFER

#include "./THeapBuffer.hpp"
#include <cstddef>

namespace tklb {
	/**
	 * @brief A handle buffer to store elements in.
	 *        Each space in the buffer has a generation counter which is increased each time a element
	 *        is removed from that spot, so the old handle becomes invalid and access is portected.
	 * @tparam T type to store
	 * @tparam Handle type to index elements
	 * @tparam GenerationBits At what bit to split the Handle for validation, defaults to 8 bits.
	 *         Higher values mean more bits are used for the generation counter to reduce the chance
	 *         of old handles still beeing valid.
	 */
	template <class T, typename Handle = unsigned int, int GenerationBits = 8, class ALLOCATOR = DefaultAllocator<>>
	class HandleBuffer {
		static constexpr Handle MaskBits = sizeof(Handle) * 8 - GenerationBits;
		static constexpr Handle MaskIndex		= (1 << MaskBits) - 1;
		static constexpr Handle MaskGeneration	= ~MaskIndex;

		struct Element {
			Handle next;				///< Linked list next element and the current generation number
			char value[sizeof(T)];		///< Space for the element
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
		 * @brief Resize buffer, does not maintain existing data or handles for now !
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

		/**
		 * @brief Get pointer for a handle
		 * @param handle Handle to look up
		 * @return T* nullptr if the handle or the element it points to is invalid.
		 */
		T* at(const Handle& handle) const {
			if (handle == InvalidHandle) { return nullptr; }

			const Handle index = handle & MaskIndex;

			TKLB_ASSERT(index != MaskIndex) // can't index this, see constructor
			if (mElements.size() <  index) { return nullptr; }

			const Element& element = mElements[index];
			const Handle currentGeneration = element.next & MaskGeneration;
			if (currentGeneration == MaskGeneration) { return nullptr; } // referencing unitialized element

			const Handle accessedGeneration = handle & MaskGeneration;
			if (accessedGeneration != currentGeneration) { return nullptr; } // referencing removed element

			return (T*) (element.value);
		}

		bool has(const Handle& handle) const { return at(handle) != nullptr; }

		inline const T& operator[](const Handle& handle) const { return *at(handle); }
		inline T& operator[](const Handle& handle) { return *at(handle); }

		/**
		 * @brief Looks for a free spot and will call the default constructor for T
		 * @return Handle A valid handle if there was space left, InvalidHandle otherwise.
		 */
		Handle create() {
			if (mStart == MaskIndex) { return InvalidHandle; } // No more space
			TKLB_ASSERT(mFree != 0) // Means there's a logic error
			const Handle index = mStart;
			auto& element = mElements[index];
			new (element.value) T();

			// remove the element from the linked list
			mStart = element.next & MaskIndex;
			mFree--;

			return index | (element.next & MaskGeneration);
		}

		/**
		 * @brief Marks a handle as invalid and allow the slot to be reused
		 * @param handle
		 * @return true The handle was valid and the elemtn is now freed
		 * @return false
		 */
		bool remove(const Handle& handle) {
			T* element = at(handle);
			if (element == nullptr) { return false; }
			element->~T();

			const Handle index = handle & MaskIndex;
			Element& slot = mElements[index];

			// Invalidate the handle by incremening the generation
			Handle generation = ((slot.next & MaskGeneration) >> MaskBits);
			generation += 1;
			if (generation == MaskGeneration) {
				// last generation is skipped because it signals a uninitialzed element
				generation = 0;
			}
			generation = (generation << MaskBits);

			// insert the free element back into the list
			slot.next = generation | index;
			mStart = index;

			mFree++;
			return true;
		}

		/**
		 * @brief Since the buffer is continuos, but might contain gaps/invalid elements
		 *        iterating needs to be done via this function.
		 * @tparam
		 * @param func Func A lambda called for each valid element
		 */
		template <class Func>
		void const iterate(Func&& func) {
			Handle itemsLeft = mElements.size() - mFree;
			for (Handle i = 0; i < mElements.size(); i++) {
				if (itemsLeft == 0) { return; } // early out
				const Element& element = mElements[i];
				if ((element.next & MaskGeneration) != MaskGeneration) {
					func(*((T*) mElements[i].value), i | (element.next & MaskGeneration));
					itemsLeft--;
				}
			}
		}
	};
}

#endif // _TKLB_HANDLE_BUFFER
