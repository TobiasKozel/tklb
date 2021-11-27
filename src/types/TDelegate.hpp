#ifndef _TKLB_DELEGATE
#define _TKLB_DELEGATE

namespace tklb {
	/**
	 * Stripped down version of this delegate implementation
	 * https://github.com/marcmo/delegates/tree/f0938dac2ad779226ef637da350e964ac8008ba5
	 * Stripped return value, const and free function version
	 * TODO tklb add return type back in
	 */

	template<typename... Parameters>
	class BaseDelegate {
	protected:
		using Callback = void (*)(void*, Parameters...);
		void* mContext; 			// Object instance
		Callback mCallback;			// Static function in factory to make the call to the correct type
	public:
		BaseDelegate(void* context, Callback func) : mContext(context), mCallback(func) { }
		void operator()(Parameters... parameters) const {
			(*mCallback)(mContext, parameters...);
		}
		bool valid() const { return mCallback != nullptr; }
	};

	template<typename... Parameters>
	struct Delegate;

	template<typename... Parameters>
	struct Delegate<void(Parameters...)> : public BaseDelegate<Parameters...> {
		using Base = BaseDelegate<Parameters...> ;
		Delegate(void* context, typename Base::Callback func) : Base(context, func) { }
		Delegate() : Base(nullptr, nullptr) { }

	};

	/**
	 * A factory is needed to ba able to convert the void pointer back
	 * to the object and call the member function
	 */
	template<typename T, typename... Parameters>
	struct DelegateFactory {
		template<void (T::*Func)(Parameters...)>
		static void Call(void* context, Parameters... parameters) {
			(static_cast<T*>(context)->*Func)(parameters...);
		}

		template<void (T::*Func)(Parameters...)>
		inline static Delegate<void(Parameters...)> Create(T* context) {
			return Delegate<void(Parameters...)>(context, &DelegateFactory::Call<Func>);
		}
	};

	template<typename T, typename... Parameters>
	DelegateFactory<T, Parameters...> MakeDelegate(void (T::*)(Parameters...)) {
		return DelegateFactory<T, Parameters...>();
	}

	#define TKLB_DELEGATE(func, thisPrtRef) (tklb::MakeDelegate(func).Create<func>(&thisPrtRef))

} // namespace tklb

#endif // _TKLB_DELEGATE
