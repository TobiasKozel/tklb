#include "../types/TBus.h"

#include "TestCommon.h"

int main() {
	{
		enum Events {
			DO = 0,
			SOME,
			THING,
			EVENT_COUNT
		};
		Bus bus(EVENT_COUNT);

		int fired = 0;

		Bus::Sub<int> sub1(bus, Events::DO, [&](int param) {
			fired++;
		});

		Bus::Sub<float> sub2(bus, Events::DO, [&](float param) {
			fired++;
		});

		bus.fire<float>(Events::DO, 123);
		bus.fire<int>(Events::DO, 123);
		bus.fire<int>(Events::SOME, 123);

		if (fired !=2 ) {
			return 1;
		}
	}

	memcheck()
	return 0;
}
