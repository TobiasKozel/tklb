#ifndef TKLBZ_SPINLOCK
#define TKLBZ_SPINLOCK

// #include <atomic>
#include "./TLockGuard.hpp"

namespace tklb {
	class SpinLock {
		// std::atomic<bool> mSpinLock;
		bool mSpinLock;
		// This should be an atomic or some kind of
		// proper sync mechanism but performance tanks with atomics
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

#endif // TKLBZ_SPINLOCK
