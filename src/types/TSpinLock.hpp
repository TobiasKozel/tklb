#ifndef _TKLB_SPINLOCK
#define _TKLB_SPINLOCK

#include "./TLockGuard.hpp"

#ifndef TKLB_NO_STDLIB
	#include <atomic>
#endif

namespace tklb {
	/**
	 * @brief Spinlock
	 * ! probably not thread safe
	 */
	class SpinLock {
		#ifndef TKLB_NO_STDLIB
			std::atomic<bool> mSpinLock;
		#else
			bool mSpinLock = false;
		#endif
	public:
		SpinLock(const SpinLock&) = delete;
		SpinLock(const SpinLock*) = delete;
		SpinLock(SpinLock&&) = delete;
		SpinLock& operator= (const SpinLock&) = delete;
		SpinLock& operator= (SpinLock&&) = delete;

		SpinLock() = default;

		using Lock = LockGuard<SpinLock>;
		using TryLock = LockGuardTry<SpinLock>;

		void lock() {
			while (mSpinLock) { };
			mSpinLock = true;
		}

		void unlock() {
			mSpinLock = false;
		}

		/**
		 * @brief Tries to lock, returns true if lock was aquired
		 */
		bool try_lock() {
			if (mSpinLock) {
				return false;
			}
			mSpinLock = true;
			return true;
		}
	};
} // namespace

#endif // _TKLB_SPINLOCK
