	// float target_rate = AudioServer::get_singleton()->get_mix_rate();
	// float global_rate_scale = AudioServer::get_singleton()->get_global_rate_scale();

	// uint64_t mix_increment = uint64_t(((get_stream_sampling_rate() * p_rate_scale) / double(target_rate * global_rate_scale)) * double(FP_LEN));

	// for (int i = 0; i < p_frames; i++) {
	// 	uint32_t idx = CUBIC_INTERP_HISTORY + uint32_t(mix_offset >> FP_BITS);
	// 	//standard cubic interpolation (great quality/performance ratio)
	// 	//this used to be moved to a LUT for greater performance, but nowadays CPU speed is generally faster than memory.
	// 	float mu = (mix_offset & FP_MASK) / float(FP_LEN);
	// 	AudioFrame y0 = internal_buffer[idx - 3];
	// 	AudioFrame y1 = internal_buffer[idx - 2];
	// 	AudioFrame y2 = internal_buffer[idx - 1];
	// 	AudioFrame y3 = internal_buffer[idx - 0];

	// 	float mu2 = mu * mu;
	// 	AudioFrame a0 = 3 * y1 - 3 * y2 + y3 - y0;
	// 	AudioFrame a1 = 2 * y0 - 5 * y1 + 4 * y2 - y3;
	// 	AudioFrame a2 = y2 - y0;
	// 	AudioFrame a3 = 2 * y1;

	// 	p_buffer[i] = (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3) / 2;

	// 	mix_offset += mix_increment;

	// 	while ((mix_offset >> FP_BITS) >= INTERNAL_BUFFER_LEN) {
	// 		internal_buffer[0] = internal_buffer[INTERNAL_BUFFER_LEN + 0];
	// 		internal_buffer[1] = internal_buffer[INTERNAL_BUFFER_LEN + 1];
	// 		internal_buffer[2] = internal_buffer[INTERNAL_BUFFER_LEN + 2];
	// 		internal_buffer[3] = internal_buffer[INTERNAL_BUFFER_LEN + 3];
	// 		if (is_playing()) {
	// 			_mix_internal(internal_buffer + 4, INTERNAL_BUFFER_LEN);
	// 		} else {
	// 			//fill with silence, not playing
	// 			for (int j = 0; j < INTERNAL_BUFFER_LEN; ++j) {
	// 				internal_buffer[j + 4] = AudioFrame(0, 0);
	// 			}
	// 		}
	// 		mix_offset -= (INTERNAL_BUFFER_LEN << FP_BITS);
	// 	}
	// }
