#ifndef _TKLB_MUTEX_DUMMY
#define _TKLB_MUTEX_DUMMY

#include "./TLockGuard.hpp"

namespace tklb {
	/**
	 * Dummy Mutex which doesn't synchronise a all
	 */
	class MutexDummy {
	public:
		MutexDummy(const MutexDummy&) = delete;
		MutexDummy(const MutexDummy*) = delete;
		MutexDummy(MutexDummy&&) = delete;
		MutexDummy& operator= (const MutexDummy&) = delete;
		MutexDummy& operator= (MutexDummy&&) = delete;
		MutexDummy() = default;
		using Lock = LockGuard<MutexDummy>;
		using TryLock = LockGuardTry<MutexDummy>;
		void lock() { }
		void unlock() { }
		bool try_lock() { return true; }
	};
} // namespace tklb

#endif // TKLB_MUTEX_DUMMY
