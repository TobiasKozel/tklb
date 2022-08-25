#ifndef _TKLB_STRING
#define _TKLB_STRING

#include "./THeapBuffer.hpp"

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
		static constexpr char Terminator = '\0';
	public:
		StackString() { for (Size i = 0; i < N; i++) { mData[i] = Terminator; } }
		StackString(const char* str) { set(str); }
		StackString& operator=(const char* str) { set(str); return *this; }

		const char* c_str() const {
			if (N == 0) { return nullptr; }
			return mData;
		}
		const char& operator[](const Size index) const {
			TKLB_ASSERT(index < N)
			if (N <= index) { return Terminator; }
			return mData[index];
		}

		char& operator[](const Size index) {
			TKLB_ASSERT(index < N)
			return mData[index];
		}

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
				if (str[i] == Terminator) {
					return str[i] == mData[i];
				}
				if (str[i] != mData[i]) { return false; }
			}
			return true;
		}

		bool empty() const {
			if (N == 0) { return true; }
			return mData[0] == Terminator;
		}

		Size size() const {
			if (N == 0) { return 0; }
			for (Size i = 0; i < N; i++) {
				if (mData[i] == Terminator) {
					return i;
				}
			}
			return N; // someone wrote past the end
		}

		void set(const char* str) {
			if (N == 0) { return; }
			Size i = 0;
			for(i = 0; i < N; i++) {
				if (str[i] == Terminator) { break; }
				mData[i] = str[i];
			}
			for(; i < N; i++) { mData[i] = Terminator; }
			mData[N - 1] = Terminator; // just in case, will run over the last char
		}
	};

	/**
	 * @brief Simple std string replacement
	 *
	 * @tparam STORAGE
	 */
	template <class STORAGE = HeapBuffer<char>>
	class String {
		STORAGE mData;
		static constexpr char Terminator = '\0';
		using Size = typename STORAGE::Size;
	public:
		static constexpr Size MAX_LENGTH = 1024 * 1024 * 32; // 32 megs
		String() { }
		String(const char* str) { set(str); }
		String(const String* str) { set(*str); };
		String(const String& str) { set(str); };
		String& operator=(const char* str) { set(str); return *this; }
		String& operator=(const String& str) { set(str); return *this; };

		const char* c_str() const { return mData.data(); }
		const char& operator[](const Size index) const { return mData[index]; }
		char& operator[](const Size index) { return mData[index]; }


		bool operator==(const char* b) const {
			// size includes the terminator
			for (Size i = 0; i < mData.size(); i++) {
				if ((*this)[i] != b[i]) {
					return false;
				}
				if (b[i] == Terminator) {
					return true; // both are equal and at the end
				}
			}
			// We have reached the terminator of this but not on b
			return false;
		}
		bool operator!=(const char* b) const { return !((*this) == b); }

		/**
		 * @brief length of string INCLUDING null terminator
		 *
		 * @return Size
		 */
		Size size() const { return mData.size(); }
		bool empty() const { return mData.empty(); }
		char* data() { return mData.data(); }

		void set(const String& str) {
			mData.set(str.c_str(), str.size());
		}

		void set(const char* str) {
			if (str == nullptr) { return; }

			Size size = 0;
			for (; size < MAX_LENGTH; size++) {
				if (str[size] == '\0') {
					// TODO check
					size++; // keep the terminator
					break;
				}
			}
			mData.resize(size);
			mData.set(str, size);
			mData[size - 1] = '\0';
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
