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
		$executable 2>&1 | tee -a "${source_file}.result.txt"
		# $executable
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

cat /proc/cpuinfo 2>&1 | grep "model name" 2>&1 > "${source_file}.result.txt"
echo "g++" 2>&1 | tee -a "${source_file}.result.txt"
test "g++ -std=c++17 -O3 -march=native"
test "g++ -std=c++17 -O3 -march=native -DTKLB_NO_SIMD"
test "g++ -std=c++17 -O3 -march=native -DTKLB_SAMPLE_FLOAT"
test "g++ -std=c++17 -O3 -march=native -DTKLB_NO_SIMD -DTKLB_SAMPLE_FLOAT"

# TODO test -ffast-math
echo "clang++" 2>&1 | tee -a "${source_file}.result.txt"
test "clang++ -std=c++17 -march=native -O3"
test "clang++ -std=c++17 -march=native -O3 -DTKLB_NO_SIMD"
test "clang++ -std=c++17 -march=native -O3 -DTKLB_SAMPLE_FLOAT"
test "clang++ -std=c++17 -march=native -O3 -DTKLB_NO_SIMD -DTKLB_SAMPLE_FLOAT"
echo ""
