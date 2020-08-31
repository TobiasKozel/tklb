#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
	Linux*)     executable=./a.out;;
	Darwin*)    executable=./a.out;;
	CYGWIN*)    executable=./a.exe;;
	MINGW*)     executable=./a.exe;;
	*)          executable=./a.out
esac

function test {
	for f in ./*.cpp
	do
		if $1 $f ; then
			$executable
			result=$?
			if [[ $result -ne 0 ]]; then
				echo "${1} ${f} Failed with ${result}"
			fi
			rm $executable
		else
			exit
		fi
	done
}

test "g++ -O2 -march=native"
test "g++ -O2 -march=native -DTKLB_NO_INTRINSICS"
# test "g++ -O2 -march=native -DTKLB_SAMPLE_FLOAT"
# test "clang++ -march=native -Ofast"
# test "clang++ -march=native -Ofast -DTKLB_NO_INTRINSICS"
# test "clang++ -march=native -Ofast -DTKLB_SAMPLE_FLOAT"
