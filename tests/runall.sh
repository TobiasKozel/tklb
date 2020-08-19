#!/bin/bash
FAILED=false
for f in ./*.cpp
do
	if g++ $f ; then
		if ./a.out ; then
			echo "Passed: ${f}"
		else
			FAILED=true
			echo -e "\e[31mError: Test failed for ${f}\e[0m"
		fi
		rm ./a.out
	else
		echo -e "\e[31mError: Failed to compile ${f}\e[0m"
		exit
	fi
done

if ($FAILED); then
	echo -e "\e[31mSome tests failed!\e[0m"
else
	echo -e "\e[32mAll tests passed!\e[0m"
fi
