#ifndef _TKLB_STRING
#define _TKLB_STRING

#include "./THeapBuffer.hpp"
#include <cstring>
namespace tklb {
	/**
	 * @brief Super simple class to hold and compare strings on the stack
	 *
	 * @tparam N Size of the string
	 */
	template <unsigned int N = 8>
	class StackString {
		char mData[N];
		using Size = unsigned int;
	public:
		StackString() { for (Size i = 0; i < N; i++) { mData[i] = '\0'; } }
		StackString(const char* str) { set(str); }
		StackString& operator=(const char* str) { set(str); return *this; }
		const char* c_str() const { return mData; }
		const char& operator[](const Size index) const { return mData[index]; }
		char& operator[](const Size index) { return mData[index]; }

		template <int N2>
		bool operator==(const StackString<N2> &b) const {
			if (N == 0) { return false; }
			for(Size i = 0; i < (N < N2 ? N : N2); i++) {
				if (mData[i] != b[i]) { return false; }
			}
			return true;
		}

		bool operator==(const char* str) const {
			if (N == 0) { return false; }
			for(Size i = 0; i < N; i++) {
				if (str[i] == '\0') {
					return str[i] == mData[i];
				}
				if (str[i] != mData[i]) { return false; }
			}
			return true;
		}

		bool empty() const {
			if (N == 0) { return true; }
			return mData[0] == '\0';
		}

		Size size() const {
			if (N == 0) { return 0; }
			for (Size i = 0; i < N; i++) {
				if (mData[i] == '\0') {
					return i;
				}
			}
			return N; // someone wrote past the end
		}

		void set(const char* str) {
			if (N == 0) { return; }
			Size i = 0;
			for(i = 0; i < N; i++) {
				if (str[i] == '\0') { break; }
				mData[i] = str[i];
			}
			for(; i < N; i++) { mData[i] = '\0'; }
			mData[N - 1] = '\0'; // just in case, will run over the last char
		}
	};

	template <class STORAGE = HeapBuffer<char>>
	class String {
		STORAGE mData;
		using Size = typename STORAGE::Size;
	public:
		String() { }
		String(const char* str) { set(str); }
		String(const String* str) { set(*str); };
		String(const String& str) { set(str); };
		String& operator=(const char* str) { set(str); return *this; }
		String& operator=(const String& str) { set(str); return *this; };

		const char* c_str() const { return mData.data(); }
		const char& operator[](const Size index) const { return mData[index]; }
		char& operator[](const Size index) { return mData[index]; }
		Size size() const { return mData.size(); }
		bool empty() const { return mData.empty(); }

		char* data() { return mData.data(); }

		void set(const String& str) {
			mData.set(str.c_str(), str.size());
		}

		void set(const char* str) {
			if (str == nullptr) { return; }
			const Size size = Size(strlen(str)) + 1;		// keep the terminator
			mData.resize(size);
			mData.set(str, size);
		}

		void append(const String& str) {
			const Size start = mData.size() - 1;	// crop off own terminator
			mData.resize(start + str.size()); 		// keeps the old data
			for (Size i = 0; i < str.size(); i++) {
				mData[start + i] = str[i];
			}
		}

		void append(const char* str) {
			String s = str;
			append(s);
		}

		void prepend(const String& str) {
			const Size oldSize = mData.size();
			mData.resize(oldSize + str.size() - 1);
			for (Size i = 0; i < oldSize; i++) {
				mData[mData.size() - i] = mData[oldSize - i];
			}
			for (Size i = 0; i < str.size() - 1; i++) { // crop off other terminator
				mData[i] = str[i];
			}
		}

		void prepend(const char* str) {
			String s = str;
			prepend(s);
		}

		void resize(Size size) { mData.resize(size); }

		void reserve(Size size) { mData.reserve(size); }
	};

} // namespace tklb

#endif // _TKLB_STRING
