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
		tklb::SizeT ret = 3;

		void add(tklb::SizeT v) {
			ret += v;
		}

		void mul(tklb::SizeT v) {
			ret *= v;
		}
	};

	Class ret;

	auto addSub = Bus::Subscription<tklb::SizeT>(
		&bus, ADD, TKLB_DELEGATE(&Class::add, ret)
	);
	auto mulSub = Bus::Subscription<tklb::SizeT>(
		&bus, MUL, TKLB_DELEGATE(&Class::mul, ret)
	);

	bus.fireEvent<tklb::SizeT>(ADD, 1);
	bus.fireEvent<tklb::SizeT>(MUL, 2);

	return ret.ret == 8 ? 0 : 1;
}
