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
} // namespace tklb
#endif // TKLBZ_LOCKGUARD
