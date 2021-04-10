#include "./TestCommon.hpp"
#include <vector>
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
	constexpr int size = sizeof(Test);
	{
		std::vector<Test> asd;
		asd.resize(2);
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
		asd.push_back(Test());
	}

	{
		tklb::HeapBuffer<Test> asd(0, 1);
		asd.resize(2);
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
		asd.push(Test());
	}

	return 0;
}
