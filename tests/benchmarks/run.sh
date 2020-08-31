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
	if $1 $source_file ; then
		$executable
		result=$?
		if [[ $result -ne 0 ]]; then
			echo "${1} ${f} Failed with ${result}"
		fi
		rm $executable
	else
		exit
	fi
}

test "g++ -O2 -march=native"
test "g++ -O2 -march=native -DTKLB_NO_INTRINSICS"
test "g++ -O2 -march=native -DTKLB_SAMPLE_FLOAT"
test "g++ -O2 -march=native -DTKLB_NO_INTRINSICS -DTKLB_SAMPLE_FLOAT"
# TODO test -ffast-math
# test "clang++ -march=native -Ofast"
# test "clang++ -march=native -Ofast -DTKLB_NO_INTRINSICS"
# test "clang++ -march=native -Ofast -DTKLB_SAMPLE_FLOAT"
# test "clang++ -march=native -Ofast -DTKLB_NO_INTRINSICS -DTKLB_SAMPLE_FLOAT"
