#include "./TestCommon.h"
#include "../types/TFile.h"

int main() {
	{
		FileInfo file("C:\\dev\\git\\tklb");
		file.scan();
		file.print();
	}
	memcheck()
	return 0;
}
