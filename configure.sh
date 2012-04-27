#!/bin/bash

if [ ! -e "build" ]
then
	echo "Creating build directory..."
	mkdir build
fi

cd build

echo "Running CMake..."
cmake ../src
