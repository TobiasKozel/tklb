#!/bin/bash

for f in ./*.cpp
do
	./test.sh $f
done

