#include "./TestCommon.hpp"
#include "../src/types/THeapBuffer.hpp"

unsigned int objectCount = 0;

class Test {
public:
	int id;
	Test() {
		id = objectCount;
		objectCount++;
	}
};

int test() {

	{
		tklb::HeapBuffer<Test, 16> asd;
		asd.resize(2);
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());

		tklb::HeapBuffer<Test> asd2(asd.data(), asd.size());
	}

	return 0;
}
