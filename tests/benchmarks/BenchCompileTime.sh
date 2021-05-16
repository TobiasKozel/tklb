#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
	Linux*)     executable=./a.out;;
	Darwin*)    executable=./a.out;;
	CYGWIN*)    executable=./a.exe;;
	MINGW*)     executable=./a.exe;;
	*)          executable=./a.out
esac

file="BenchCompileTime.sh.result.txt"

function test {
	TIMEFORMAT="%R seconds ${1}"
	(time {
		$1 ./BenchCompileTime.cc >> /dev/null 2>&1
	}) 2>&1 | tee -a $file
	# echo "g++" 2>&1 | tee "${source_file}.result.txt"
	rm $executable
}
echo "Benchmark compile times" 2>&1 | tee $file
test "g++ -march=native"
test "g++ -march=native -DTKLB_NO_SIMD"
test "g++ -O2 -march=native"
test "g++ -O2 -march=native -DTKLB_NO_SIMD"
test "clang++ -march=native"
test "clang++ -march=native -DTKLB_NO_SIMD"
test "clang++ -march=native -Ofast"
test "clang++ -march=native -Ofast -DTKLB_NO_SIMD"
echo ""
