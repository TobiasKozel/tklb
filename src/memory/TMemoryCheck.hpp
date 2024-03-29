#ifndef _TKLB_MEMORY_CHECK
#define _TKLB_MEMORY_CHECK

#include "../types/TTypes.hpp"
#include "./TNew.hpp"

#ifndef TKLB_ASSERT
	#define TKLB_ASSERT(statement)
#endif

namespace tklb { namespace memory { namespace check {
	/**
	 * @brief Magic string at the start of each allocation
	 *        to detect corruption. 15 bytes so the magic block is 24 bytes total.
	 */
	constexpr char MAGIC_STRING_START[] = "startblockstri";

	constexpr char MAGIC_STRING_END[sizeof(MAGIC_STRING_START)] = "endblockstring";

	/**
	 * @brief Struct inserted at the end and start of every allocation
	 */
	class MagicBlock {
		SizeT size;								///< Original requested size, just to keep track of allocated space
		char magic[sizeof(MAGIC_STRING_START)];		///< magic block to detect heap corruption.
		bool start;									///< Whether the block is at the start or end of the allocation.

	public:
		MagicBlock(SizeT _size, bool _start) {
			size = _size;
			start = _start;
			if (start) {
				for (SizeT i = 0; i < sizeof(MAGIC_STRING_START); i++) {
					magic[i] = MAGIC_STRING_START[i];
				}
			} else {
				for (SizeT i = 0; i < sizeof(MAGIC_STRING_START); i++) {
					magic[i] = MAGIC_STRING_END[i];
				}
			}
		}

		/**
		 * @brief Just the MagicBlock size times 2 to sandwhich the actual allocation
		 */
		static SizeT sizeNeeded() { return 2 * sizeof(MagicBlock); }

		/**
		 * @brief Constructs the surounding magic blocks and offsets the pointer
		 *
		 * @param allocation The allocation with extra space according to sizeNeeded()
		 * @param bytes The actual size of the usable space, not including sizeNeeded()!
		 * @return void* Offset pointer to useable space
		 */
		static void* construct(void* allocation, SizeT bytes) {
			MagicBlock* startBlock = new (allocation) MagicBlock(bytes, true);
			void* useableSpace = startBlock + 1;
			void* endBlock = ((char*) useableSpace) + bytes;
			new (endBlock) MagicBlock(bytes, false);
			return useableSpace;
		}

		struct CheckResult {
			void* ptr;				///< Pointer of the real allocation
			SizeT size = 0;			///< Size of the allocation if recoverable
			bool underrun = true;	///< start block is corrupt (can't recover end block)
			bool overrun = true;	///< end block is corrupt
		};

		/**
		 * @brief Check block
		 * @param ptr Pointer to usable space
		 */
		static CheckResult check(void* ptr) {
			CheckResult result;
			MagicBlock* startBlock = ((MagicBlock*) ptr) - 1;
			result.ptr = startBlock;


			if (startBlock->checkMagicString()) {
				// Only check the end block after it's safe to assume the first one isn't corrupt
				MagicBlock* endBlock = (MagicBlock*) ((char*) ptr + startBlock->size);
				result.underrun = false;

				if (endBlock->checkMagicString()) {
					if (startBlock->size != endBlock->size) {
						// Could mean overrun or underrun since we don't know which size is correct.
						return result;
					}
					result.overrun = false;
					result.size = startBlock->size;
				}
			}

			return result;
		}

	private:
		/**
		 * @brief Compare two strings
		 */
		static inline bool compare(const char* a, const char* b, SizeT s) {
			for (SizeT i = 0; i < s; i++) {
				if (a[i] != b[i]) {
					return false;
				}
			}
			return true;
		}

		/**
		 * @brief checks if the allocation is still intact
		 */
		bool checkMagicString() {
			if (start) {
				if (!compare(magic, MAGIC_STRING_START, sizeof(MAGIC_STRING_START))) {
					return false;
				}
			} else {
				if (!compare(magic, MAGIC_STRING_END, sizeof(MAGIC_STRING_END))) {
					return false;
				}
			}
			return true;
		}
	}; // struct MagicBlock

} } } // namespace memory::namespace::check

#endif // _TKLB_MEMORY_CHECK
