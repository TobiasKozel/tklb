#include "../util/TLeakChecker.h"

struct Test {
	int a;
	int b;
};

int main() {
	Test* test = new Test();
	delete test;
	auto test2 = new Test[10];
	delete[] test2;
	return tklb::allocationCount;
}
