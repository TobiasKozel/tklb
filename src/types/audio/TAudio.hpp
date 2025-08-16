#ifndef _TKLB_AUDIO
#define _TKLB_AUDIO

#include "../../util/TMath.hpp"

namespace tklb {

	template <typename T>
	static T dbToGain (T db, T silenceDb = -100) {
		if (db < silenceDb) {
			return 0;
		}
		return pow (T(10), db * T(0.05));
	}

	template <typename T>
	static T gainToDb (T gain, T silenceDb = -100) {
		if (gain < T(0)) {
			return silenceDb;
		}
		return max(log10(gain) * T(20), silenceDb);
	}

}

#endif // _TKLB_AUDIO
