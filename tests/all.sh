#!/bin/bash

FAILED=false
uname -a
echo ""
g++ -v
echo ""
clang++ -v
echo ""

for f in ./*.cpp
do
	./run.sh $f
	if [[ $? -ne 0 ]] ; then
		FAILED=true
	fi
done

if ($FAILED); then
	echo -e "\n\e[31mSome tests failed!\e[0m"
	exit 1
else
	echo -e "\n\e[32mAll tests passed!\e[0m"
fi
