#ifndef _TKLB_EVENT_BUS
#define _TKLB_EVENT_BUS

#include "./THeapBuffer.hpp"

#include "./TMutexDummy.hpp"
#include "./TDelegate.hpp"

#ifndef TKLB_ASSERT
	#define TKLB_ASSERT(cond)
#endif

namespace tklb {
	/**
	 *
	 */
	template <int EVENT_COUNT, typename EventId = int, class MutexType = MutexDummy>
	class EventBus {
		class BaseSubscription;
		using Subscriptions = HeapBuffer<BaseSubscription*>;

		Subscriptions mEvents[EVENT_COUNT];
		MutexType mMutex;
		using Lock = typename MutexType::Lock;

		void addSubscriber(BaseSubscription* sub, const EventId eventId) {
			TKLB_ASSERT(eventId < EVENT_COUNT)
			Lock lock(mMutex);
			mEvents[eventId].push(sub);
		}

		void removeSubscriber(BaseSubscription* sub, const EventId eventId) {
			TKLB_ASSERT(eventId < EVENT_COUNT)
			Lock lock(mMutex);
			mEvents[eventId].remove(sub);
		}

		class BaseSubscription {
		public:
			virtual ~BaseSubscription() { };
		};

	public:
		EventBus() { }
		~EventBus() { }

		template<typename... Parameters>
		class Subscription : public BaseSubscription {
			friend EventBus;
			using Callback = Delegate<void(Parameters...)>;
			const Callback mCallback;
			EventBus* const mBus = nullptr;
			const EventId mEventId = 0; // only needed for unsubbing, could be omitted
		public:
			Subscription(EventBus*, const EventId, const Callback&&);
			~Subscription();
		};

		template<typename... Parameters>
		void fireEvent(const EventId eventId, Parameters... param) {
			TKLB_ASSERT(eventId < EVENT_COUNT)
			if (mEvents[eventId].size() == 0) { return; } // don't even wait for lock
			Lock lock(mMutex);
			for (typename Subscriptions::Size i = 0; i < mEvents[eventId].size(); i++) {
				#ifdef TKLB_MEMORY_CHECK
					auto sub = dynamic_cast<Subscription<Parameters...>*>(mEvents[eventId][i]);
					TKLB_ASSERT(sub != nullptr)
				#else
					auto sub = reinterpret_cast<Subscription<Parameters...>*>(mEvents[eventId][i]);
				#endif // TKLB_MEMORY_CHECK
				sub->mCallback(param...);
			}
		}
	}; // class EventBus

	template <int EVENT_COUNT, typename EventId, class MutexType>
	template<typename... Parameters>
	EventBus<EVENT_COUNT, EventId, MutexType>::Subscription<Parameters...>::~Subscription() {
		Subscription::mBus->removeSubscriber(this, Subscription::mEventId);
	}

	template <int EVENT_COUNT, typename EventId, class MutexType>
	template<typename... Parameters>
	EventBus<EVENT_COUNT, EventId, MutexType>::Subscription<Parameters...>::Subscription(
		EventBus* bus, const EventId eventId, const Callback&& callback
	) : mCallback(callback), mBus(bus), mEventId(eventId) {
		TKLB_ASSERT(bus != nullptr)
		bus->addSubscriber(this, eventId);
	}
} // namespace tklb

#endif // _TKLB_EVENT_BUS
