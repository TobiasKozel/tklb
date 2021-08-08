			// float target_rate = AudioServer::get_singleton()->get_mix_rate();
			// float global_rate_scale = AudioServer::get_singleton()->get_global_rate_scale();

			// uint64_t mix_increment = uint64_t(((get_stream_sampling_rate() * p_rate_scale) / double(target_rate * global_rate_scale)) * double(FP_LEN));

			// for (int i = 0; i < p_frames; i++) {
			// 	uint32_t idx = CUBIC_INTERP_HISTORY + uint32_t(mix_offset >> FP_BITS);
			// 	// 4 point, 4th order optimal resampling algorithm from: http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf
			// 	float mu = (mix_offset & FP_MASK) / float(FP_LEN);
			// 	AudioFrame y0 = internal_buffer[idx - 3];
			// 	AudioFrame y1 = internal_buffer[idx - 2];
			// 	AudioFrame y2 = internal_buffer[idx - 1];
			// 	AudioFrame y3 = internal_buffer[idx - 0];

			// 	AudioFrame even1 = y2 + y1, odd1 = y2 - y1;
			// 	AudioFrame even2 = y3 + y0, odd2 = y3 - y0;
			// 	AudioFrame c0 = even1 * 0.46835497211269561 + even2 * 0.03164502784253309;
			// 	AudioFrame c1 = odd1 * 0.56001293337091440 + odd2 * 0.14666238593949288;
			// 	AudioFrame c2 = even1 * -0.250038759826233691 + even2 * 0.25003876124297131;
			// 	AudioFrame c3 = odd1 * -0.49949850957839148 + odd2 * 0.16649935475113800;
			// 	AudioFrame c4 = even1 * 0.00016095224137360 + even2 * -0.00016095810460478;
			// 	p_buffer[i] = (((c4 * mu + c3) * mu + c2) * mu + c1) * mu + c0;

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
