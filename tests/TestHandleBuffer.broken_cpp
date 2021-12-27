#include "./TestCommon.hpp"
#include "../src/types/THandleBuffer.hpp"

int Index = 0;

struct Test {
	Handle id;
	char test;
	Test() {
		id = Index;
		Index++;
		test = 'z';
	}

	Test(char text) {
		id = Index;
		Index++;
		test = text;
	}

	~Test() {
		int i = 0;
	}
};

int test() {
	HandleBuffer<Test> handles(&Test::id);
	Test a = 'a';
	Test b = 'b';
	Test c = 'c';
	Test d = 'd';
	Handle ah = handles.push(a);
	Handle bh = handles.push(b);
	Handle ch = handles.push(c);
	Test ref;
	handles.pop(bh, &ref);
	if (ref.test != 'b') { return 1; }
	Test* ac = handles.at(ah);
	if (ac->test != 'a') { return 2; }
	Test* cc = handles.at(ch);
	if (cc->test != 'c') { return 3; }
	cc->id = 4; // change id
	cc = handles.at(ch);
	if (cc != nullptr) { return 4; }
	Handle dh = handles.push(d);
	return 0;
}
