#ifndef TKLB_POINTERLIST
#define TKLB_POINTERLIST

#include <vector>
#include <algorithm>

namespace tklb {
template <class T>
/**
 * Holds a list of pointers
 * Wraps around a std::vector for now
 * TODO replace vector with heapbuffer
 */
class PointerList {
	std::vector<T*> mList;

public:
	T* get(const size_t index) const {
		if (index >= mList.size()) {
			return nullptr;
		}
		return mList.at(index);
	}

	T* operator[](const size_t index) const {
		return get(index);
	}

	void add(T* element) {
		mList.push_back(element);
	}

	void remove(const T* element, bool const destroy = false) {
		auto position = std::find(mList.begin(), mList.end(), element);
		if (position != mList.end()) {
			mList.erase(position);
		}
		if (destroy) {
			delete element;
		}
	}

	void remove(const size_t index, bool const destroy = false) {
		remove(get(index), destroy);
	}

	size_t size() const {
		return mList.size();
	}

	int find(const T* element) const {
		auto position = std::find(mList.begin(), mList.end(), element);
		if (position != mList.end()) {
			return static_cast<int>(std::distance(mList.begin(), position));
		}
		return -1;
	}

	void clear(const bool destroy = false) {
		if (destroy) {
			for (int i = 0; i < size(); i++) {
				delete get(i);
			}
		}
		mList.clear();
	}

	void insert(T* element, const int index) {
		if (index > size() || index < 0) { return; }
		mList.insert(mList.begin() + index, element);
	}
};

} // namespace

#endif // TKLB_POINTERLIST