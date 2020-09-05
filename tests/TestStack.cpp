#include "TestCommon.h"
#include "../types/TStack.h"

int main() {
	PointerStack<int, 1024> stack;

	int a = 100;
	int b = 200;
	int c = 300;

	stack.push(a);
	stack.push(b);
	stack.push(c);

	if (stack.length() != 3) {
		return 1;
	}
	if (*stack.pop() != 300) {
		return 2;
	}

	if (*stack.pop() != 200) {
		return 3;
	}

	if (*stack.pop() != 100) {
		return 4;
	}

	if (stack.length() != 0) {
		return 5;
	}

	return 0;

}
