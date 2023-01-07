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

exit_code=0

function test {
	if $1 $source_file ; then
		$executable
		result=$?
		if [[ $result -eq 0 ]] ; then
			echo -ne "\r\033[2KPassed: ${1} ${source_file}"
		else
			echo -e "\n\e[31m${1} ${source_file}\e[0m"
			echo -e "\n\e[31mError: Test failed with result ${result}\e[0m"
			exit_code=$result
		fi
		rm $executable
	else
		echo -e "\n\e[31m${1} ${source_file}\e[0m"
		echo -e "\n\e[31mError: Failed to compile\e[0m"
		exit 222
	fi
}

echo "Test ${source_file}"
test "g++ -std=c++14 -O3 -Wall -march=native"
# xsimd causes a few warnings, so predantic os only enabledwithout simd
test "g++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_NO_SIMD"
test "g++ -std=c++14 -O3 -Wall -march=native -DTKLB_SAMPLE_FLOAT"
test "g++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_SAMPLE_FLOAT -DTKLB_NO_SIMD"
test "g++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_SAMPLE_FLOAT -DTKLB_NO_SIMD -DTKLB_NO_STDLIB"
test "clang++ -std=c++14 -O3 -Wall -march=native"
test "clang++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_NO_SIMD"
test "clang++ -std=c++14 -O3 -Wall -march=native -DTKLB_SAMPLE_FLOAT"
test "clang++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_SAMPLE_FLOAT -DTKLB_NO_SIMD"
test "clang++ -std=c++14 -O3 -Wall -pedantic -Wextra -march=native -DTKLB_SAMPLE_FLOAT -DTKLB_NO_SIMD -DTKLB_NO_STDLIB"

# TODO figure out how to get into dev shell for windows
#if [ "$executable" == "./a.exe" ]; then
#	cmd /C "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
#	test "cl /O2 /arch:AVX"
#	test "cl /O2 /arch:AVX /DTKLB_NO_SIMD"
#	test "cl /O2 /arch:AVX /DTKLB_SAMPLE_FLOAT"
#fi

exit $exit_code
