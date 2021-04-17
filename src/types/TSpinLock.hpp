#ifndef TKLBZ_SPINLOCK
#define TKLBZ_SPINLOCK

#include <atomic>

namespace tklb {
	class SpinLock {
		std::atomic<bool> mSpinLock;
	public:
		SpinLock(const SpinLock&) = delete;
		SpinLock(const SpinLock*) = delete;
		SpinLock(SpinLock&&) = delete;
		SpinLock& operator= (const SpinLock&) = delete;
		SpinLock& operator= (SpinLock&&) = delete;

		SpinLock() = default;

		void lock() {
			while (mSpinLock) { };
			mSpinLock = true;
		}

		void unlock() {
			mSpinLock = false;
		}
	};
} // namespace

#endif // TKLBZ_SPINLOCK
