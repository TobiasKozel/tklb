#ifndef TKLBZ_MUTEX
#define TKLBZ_MUTEX

#include <mutex>
#include "./TLockGuard.hpp"

namespace tklb {
	/**
	 * @brief Mutex wrapper
	 */
	class Mutex {
		std::mutex mMutex;
	public:
		Mutex(const Mutex&) = delete;
		Mutex(const Mutex*) = delete;
		Mutex(Mutex&&) = delete;
		Mutex& operator= (const Mutex&) = delete;
		Mutex& operator= (Mutex&&) = delete;

		Mutex() = default;

		using Lock = LockGuard<Mutex>;
		using TryLock = LockGuardTry<Mutex>;

		void lock() {
			mMutex.lock();
		}

		void unlock() {
			mMutex.unlock();
		}

		/**
		 * @brief Tries to lock, returns true if lock was aquired
		 */
		bool try_lock() {
			return mMutex.try_lock();
		}
	};
} // namespace

#endif // TKLB_MUTEX
