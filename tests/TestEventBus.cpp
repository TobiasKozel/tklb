#include "./TestCommon.hpp"
#include "../src/types/TEventBus.hpp"

int test() {
	enum EventList {
		ADD,
		MUL,
		TOTAL_COUNT
	};

	using Bus = tklb::EventBus<TOTAL_COUNT, EventList>;

	Bus bus;

	struct Class {
		size_t ret = 3;

		void add(size_t v) {
			ret += v;
		}

		void mul(size_t v) {
			ret *= v;
		}
	};

	Class ret;

	auto addSub = Bus::Subscription<size_t>(
		&bus, ADD, TKLB_DELEGATE(&Class::add, ret)
	);
	auto mulSub = Bus::Subscription<size_t>(
		&bus, MUL, TKLB_DELEGATE(&Class::mul, ret)
	);

	bus.fireEvent<size_t>(ADD, 1);
	bus.fireEvent<size_t>(MUL, 2);

	return ret.ret == 8 ? 0 : 1;
}
