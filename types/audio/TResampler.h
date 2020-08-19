#ifndef TKLB_RESAMPLER
#define TKLB_RESAMPLER

#include "../../external/SpeexResampler.h"
namespace tklb {

class Resampler {
	speexport::SpeexResampler resampler;
public:
	Resampler(int rateIn, int rateOut, int channels = 2, int quality = 5) {
		int err;
		resampler.init(channels, rateIn, rateOut, quality, &err);
	}

};

} // namespace

#endif // RESAMPLER
