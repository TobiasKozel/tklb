#ifndef TKLB_STACK
#define TKLB_STACK

namespace tklb {

template <class T, int SIZE>
class PointerStack {
	T* mStack[SIZE] = { nullptr };
	int mIndex = 0;

public:
	bool push(T* i) {
		if (mIndex < SIZE) {
			mStack[mIndex] = i;
			mIndex++;
			return true;
		}
		return false;
	}

	bool push(T& i) {
		return push(&i);
	}

	T* pop() {
		if (mIndex > 0) {
			mIndex--;
			return mStack[mIndex];
		}
		return nullptr;
	}

	int length() const {
		return mIndex;
	}

	bool empty() const {
		return mIndex == 0;
	}

	bool full() const {
		return mIndex == SIZE;
	}
};

} // namespace
#endif // TKLB_STACK