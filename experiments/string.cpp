#include "./ExperimentsCommon.hpp"
#include "../src/types/THeapBuffer.hpp"

using String = HeapBuffer<char>;

struct File {
	String name;
	size_t size;
};

int main() {
	HeapBuffer<File> files;
	{
		File f1;
		f1.name.set("testname", 9);
		files.push(f1);
	}
	File& f1 = files[0];
	auto& buf = f1.name;
	int as = 0;
	return 0;
}
