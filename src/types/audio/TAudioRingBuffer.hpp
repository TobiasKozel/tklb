#ifndef _TKLB_AUDIORINGBUFFER
#define _TKLB_AUDIORINGBUFFER

#include "./TAudioBuffer.hpp"

namespace tklb {
	template <typename T, class STORAGE = HeapBuffer<T, DEFAULT_ALIGNMENT_BYTES>>
	class AudioRingBufferTpl : public AudioBufferTpl<T, STORAGE> {
		using Base = AudioBufferTpl<T, STORAGE>;
		using uchar = unsigned char;
		using Size = typename Base::Size;
		Size mHead = 0;
		Size mTail = 0;

	public:
		AudioRingBufferTpl() { }

		AudioRingBufferTpl(const Size length, const uchar channels)
			: AudioBufferTpl<T, STORAGE>(length, channels) { }

		void reset() {
			Base::set(0.0);
			mTail = mHead = 0;
		}

		/**
		 * @brief Puts a number of elements in the buffer provided
		 * @param out Destination buffer to store retrieved samples in
		 * @param elements How many elements to retrieve from the ring buffer
		 * @param offsetSrc Where to start in the ringbuffer
		 * @param offsetDst Where to start in the destination buffer
		 * @return How many elements where retrieved
		 */
		template <typename T2, class STORAGE2>
		Size peek(AudioBufferTpl<T2, STORAGE2>& out, Size elements, Size offsetSrc = 0, Size offsetDst = 0) {
			const Size head = mHead - offsetSrc; // Offset the head
			if (elements > head) {
				elements = head; // Clamp the elements to peek to the elements in the buffer
			}
			if (elements > 0) {
				Size tailStart = mTail - head; // This should always be negative when the offset is 0
				if (mTail < head) {
					// So move it back
					// TODO tklb this does overflow, which is a little mad
					tailStart += Base::size();
				}
				const Size spaceLeft = Base::size() - tailStart;
				if (spaceLeft < elements) {
					// Means it wraps around and split in two moves
					out.set(*this, spaceLeft             , tailStart, offsetDst);
					out.set(*this, (elements - spaceLeft), 0        , spaceLeft + offsetDst);
				}
				else {
					// Enough buffer left and can be done in one step
					out.set(*this, elements, tailStart, offsetDst);
				}
			}
			out.setValidSize(elements);
			return elements;
		}

		/**
		 * @brief Pops a number of elements in the buffer provided
		 * @param out Destination buffer to store retrieved samples in
		 * @param elements How many elements to retrieve from the ring buffer
		 * @param offsetSrc Where to start in the ringbuffer
		 * @param offsetDst Where to start in the destination buffer
		 * @return How many elements where retrieved
		 */
		template <typename T2, class STORAGE2>
		Size pop(AudioBufferTpl<T2, STORAGE2>& out, const Size elements, Size offsetSrc = 0, Size offsetDst = 0) {
			const Size elementsOut = peek(out, elements, offsetSrc, offsetDst);
			mHead -= elementsOut; // Move the head back, can't exceed bounds since it was clamped in peek
			return elementsOut;
		}

		/**
		 * @brief Adds validSize() amount of frames to the buffer
		 * @param in Source buffer
		 * @param offsetSrc Where to start in the source buffer
		 * @return How many elements where stored in the ring buffer
		 */
		template <typename T2, class STORAGE2>
		Size push(const AudioBufferTpl<T2, STORAGE2>& in, Size offsetSrc = 0) {
			const Size spaceLeftHead = Base::size() - mHead; // Space left before exceeding upper buffer bounds
			Size elements = in.validSize();
			if (elements > spaceLeftHead) {
				/**
				 * Clamp the elements added to the buffer to it's bounds
				 * This ring buffer will stop adding elements if that's the case instead of wrapping around
				 */
				elements = spaceLeftHead;
			}
			if (elements > 0) {
				const Size spaceLeftTail = Base::size() - mTail;
				if (spaceLeftTail < elements) {
					// We'll need to wrap around since there's not enough space left
					Base::set(in, spaceLeftTail             , 0 + offsetSrc            , mTail);
					Base::set(in, (elements - spaceLeftTail), spaceLeftTail + offsetSrc);
					mTail = elements - spaceLeftTail;
				}
				else { // If there's enough space left we can just copy the buffer starting at the tail index
					Base::set(in, elements, offsetSrc, mTail);
					// If we end up taking all space left for the tail, move it back to the start, otherwise move it forward
					mTail = (spaceLeftTail == elements) ? 0 : mTail + elements;
				}
				mHead += elements; // Move the head forward, can't exceed the bounds since we clamped it
			}
			return elements;
		}

		/**
		 * @brief Returns how many more elements the buffer can hold
		 */
		Size remaining() const { return Base::size() - mHead; }

		/**
		 * @brief Returns how many elements are in the buffer
		 */
		Size filled() const { return mHead; }
	};

	using AudioRingBufferFloat = AudioRingBufferTpl<float>;
	using AudioRingBufferDouble = AudioRingBufferTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using AudioRingBuffer = AudioRingBufferTpl<float>;
	#else
		using AudioRingBuffer = AudioRingBufferTpl<double>;
	#endif
}
#endif // _TKLB_AUDIORINGBUFFER
