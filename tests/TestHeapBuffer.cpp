#include "./TestCommon.hpp"
#include "../src/types/THeapBuffer.hpp"

template <class T>
using HeapBufferType = tklb::HeapBuffer<T>;

int ObjectCount = 0;
int ConstructorCount = 0;

class LifeCycleTest {
public:
	int id;
	int payload = 0;
	LifeCycleTest() {
		id = ObjectCount;
		ObjectCount++;
		ConstructorCount++;
	}

	~LifeCycleTest() {
		ObjectCount--;
	}
};

template <class Buffer>
int runTest() {
	{
		Buffer heap;
		{ // Test resize reserve
			heap.reserve(4);
			if (heap.size() != 0) {
				return 1;
			}
			heap.resize(0);
			if (heap.size() != 0) {
				return 2;
			}
			heap.resize(2);
			if (heap.size() != 2) {
				return 3;
			}
			if (heap[0].id != 0) {
				return 4;
			}
			if (heap[1].id != 1) {
				return 5;
			}
			if (ObjectCount != 2) {
				return 6;
			}
		}


		{ // Test push pop
			LifeCycleTest push;
			push.payload = 1337;
			heap.push(push);

			LifeCycleTest pop;
			heap.pop(&pop);
			if (pop.payload != 1337) {
				return 7;
			}
		}

		if (ObjectCount != 2) {
			return 8;
		}

		{
			LifeCycleTest* push = TKLB_NEW(LifeCycleTest);
			LifeCycleTest* pop = TKLB_NEW(LifeCycleTest);
			heap.push(*push);
			heap.pop(pop);
			TKLB_DELETE(LifeCycleTest, push);
			TKLB_DELETE(LifeCycleTest, pop);
		}
	}

	if (ObjectCount != 0) {
		return 8;
	}

	return 0;
}

int test() {
	int res = runTest<tklb::HeapBuffer<LifeCycleTest>>();
	if (res) return res;
	res = runTest<tklb::HeapBuffer<LifeCycleTest, 16>>();
	if (res) return res;
	res = runTest<tklb::HeapBuffer<LifeCycleTest, 32, tklb::DefaultAllocator<unsigned char>, tklb::SizeT>>();
	return res;
}
