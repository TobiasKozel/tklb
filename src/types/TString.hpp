#ifndef _TKLB_STRING
#define _TKLB_STRING

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
		StackString() { for (auto& i : mData) { i = '\0'; } }
		StackString(const char* str) { set(str); }
		StackString& operator=(const char* str) { set(str); return *this; }
		const char* c_str() const { return mData; }
		const char& operator[](const Size index) const { return mData[index]; }
		char& operator[](const Size index) { return mData[index]; }

		template <int N2>
		bool operator==(const StackString<N2> &b) const {
			for(Size i = 0; i < (N < N2 ? N : N2); i++) {
				if (mData[i] != b[i]) { return false; }
			}
			return true;
		}

		bool operator==(const char* str) const {
			for(Size i = 0; i < N; i++) {
				if (str[i] == '\0') { break; }
				if (str[i] != mData[i]) { return false; }
			}
			return true;
		}

		bool empty() const { return mData[0] == '\0'; }

		bool size() const {
			for (Size i = 0; i < N; i++) {
				if (mData[i] == '\0') {
					return i;
				}
			}
			return N; // someone wrote past the end
		}

		void set(const char* str) {
			Size i = 0;
			for(i = 0; i < N; i++) {
				if (str[i] == '\0') { break; }
				mData[i] = str[i];
			}
			for(; i < N; i++) { mData[i] = 0; }
			mData[N - 1] = '\n'; // always terminate
		}
	};

} // namespace tklb

#endif // _TKLB_STRING
