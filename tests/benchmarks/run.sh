#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
	Linux*)     executable=./a.out;;
	Darwin*)    executable=./a.out;;
	CYGWIN*)    executable=./a.exe;;
	MINGW*)     executable=./a.exe;;
	*)          executable=./a.out
esac

source_file=$1

function test {
	if $1 $source_file >> /dev/null 2>&1 ; then
		$executable
		result=$?
		if [[ $result -ne 0 ]]; then
			echo "${1} ${source_file} Failed with ${result}"
		fi
		rm $executable
	else
		echo -e "\e[31m${1} Error: Failed to compile ${source_file}\e[0m"
		exit
	fi
}
echo "g++"
test "g++ -std=c++17 -O2 -march=native"
test "g++ -std=c++17 -O2 -march=native -DTKLB_NO_SIMD"
test "g++ -std=c++17 -O2 -march=native -DTKLB_SAMPLE_FLOAT"
test "g++ -std=c++17 -O2 -march=native -DTKLB_NO_SIMD -DTKLB_SAMPLE_FLOAT"
echo "clang++"
# TODO test -ffast-math
test "clang++ -std=c++17 -march=native -Ofast"
test "clang++ -std=c++17 -march=native -Ofast -DTKLB_NO_SIMD"
test "clang++ -std=c++17 -march=native -Ofast -DTKLB_SAMPLE_FLOAT"
test "clang++ -std=c++17 -march=native -Ofast -DTKLB_NO_SIMD -DTKLB_SAMPLE_FLOAT"
echo ""
