#include "./TestCommon.hpp"
#include "../src/types/THandleBuffer.hpp"

int Index = 0;

struct Test {
	Handle id;
	Test() {
		id = Index;
		Index++;
	}

	~Test() {
		int i = 0;
	}
};

int test() {
	HandleBuffer<Test> handles(&Test::id);
	Test a;
	handles.push(a);
	return 0;
}
