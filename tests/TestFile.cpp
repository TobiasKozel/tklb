#include "./TestCommon.hpp"
#include "../src/types/TFile.hpp"

int test() {
	FileInfo file("./");
	file.scan();
	return 0;
}
