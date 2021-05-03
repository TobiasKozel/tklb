#ifndef TKLBZ_LOCKGUARD
#define TKLBZ_LOCKGUARD

namespace tklb {
	template <class T>
	class LockGuard {
		T& mMutex;
	public:
		LockGuard(const LockGuard&) = delete;
		LockGuard(const LockGuard*) = delete;
		LockGuard(LockGuard&&) = delete;
		LockGuard& operator= (const LockGuard&) = delete;
		LockGuard& operator= (LockGuard&&) = delete;

		LockGuard(T& mutex) : mMutex(mutex) {
			mutex.lock();
		}

		~LockGuard() {
			mMutex.unlock();
		}
	};

	template <class T>
	class LockGuardTry {
		T& mMutex;
		bool mGotLock = false;
	public:
		LockGuardTry(const LockGuardTry&) = delete;
		LockGuardTry(const LockGuardTry*) = delete;
		LockGuardTry(LockGuardTry&&) = delete;
		LockGuardTry& operator= (const LockGuardTry&) = delete;
		LockGuardTry& operator= (LockGuardTry&&) = delete;

		LockGuardTry(T& mutex) : mMutex(mutex) {
			mGotLock = mutex.try_lock();
		}

		bool isLocked() const {
			return mGotLock;
		}

		~LockGuardTry() {
			if (mGotLock) {
				mMutex.unlock();
			}
		}
	};
} // namespace tklb
#endif // TKLBZ_LOCKGUARD
