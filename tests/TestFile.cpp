#include "./TestCommon.hpp"
#include "../types/TFile.hpp"

int test() {
	FileInfo file("./");
	file.scan();
	return 0;
}
