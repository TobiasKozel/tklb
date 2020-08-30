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
			if $executable ; then
				echo "${1} Passed: ${f}"
			else
				FAILED=true
				echo -e "\e[31m${1}  Error: Test failed for ${f}\e[0m"
			fi
			rm $executable
		else
			echo -e "\e[31m${1} Error: Failed to compile ${f}\e[0m"
			exit
		fi
	done
}
test g++ -O2
test clang++

if ($FAILED); then
	echo -e "\e[31mSome tests failed!\e[0m"
else
	echo -e "\e[32mAll tests passed!\e[0m"
fi
