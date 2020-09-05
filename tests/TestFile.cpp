#include "./TestCommon.h"
#include "../types/TFile.h"

int main() {
	{
		FileInfo file("./");
		file.scan();
		file.print();
	}
	memcheck()
	return 0;
}
