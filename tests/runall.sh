#!/bin/bash
FAILED=false

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
			if [[ $result -eq 0 ]] ; then
				echo "${1} Passed: ${f}"
			else
				FAILED=true
				echo -e "\e[31m${1}  Error: Test failed for ${f}. Returned ${result}\e[0m"
			fi
			rm $executable
		else
			echo -e "\e[31m${1} Error: Failed to compile ${f}\e[0m"
			exit
		fi
	done
}

test "g++ -O2 -march=native"
test "g++ -O2 -march=native -DTKLB_NO_INTRINSICS"
test "g++ -O2 -march=native -DTKLB_SAMPLE_FLOAT"
test "g++ -O2 -march=native -DTKLB_SAMPLE_FLOAT -DTKLB_NO_INTRINSICS"
test "clang++ -march=native -Ofast"
test "clang++ -march=native -Ofast -DTKLB_NO_INTRINSICS"
test "clang++ -march=native -Ofast -DTKLB_SAMPLE_FLOAT"
test "clang++ -march=native -Ofast -DTKLB_SAMPLE_FLOAT -DTKLB_NO_INTRINSICS"

# TODO figure out how to get into dev shell for windows
# if [ "$executable" == "./a.exe" ]; then
# 	test "cl /O2 /arch:AVX"
# 	test "cl /O2 /arch:AVX /DTKLB_NO_INTRINSICS"
# 	test "cl /O2 /arch:AVX /DTKLB_SAMPLE_FLOAT"
# fi

if ($FAILED); then
	echo -e "\e[31mSome tests failed!\e[0m"
else
	echo -e "\e[32mAll tests passed!\e[0m"
fi
