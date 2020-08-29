#!/bin/bash
FAILED=false

function test {
	for f in ./*.cpp
	do
		if $1 $f ; then
			if ./a.out ; then
				echo "${1} Passed: ${f}"
			else
				FAILED=true
				echo -e "\e[31m${1}  Error: Test failed for ${f}\e[0m"
			fi
			rm ./a.out
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
