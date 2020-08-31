#include "../../types/audio/TAudioBuffer.h"
#include "BenchmarkCommon.h"

int main() {
	AudioBuffer buffer;
	/**
	 * Do some benchmarking
	 */
	{
		TIMER(AudioBuffer add, Microseconds);
		// for(int i = 0; i < 10000; i++) {
		// 	buffer.add(buffer2);
		// }
	}
	return 0;
}
