#!/bin/bash

# Set the environment variable to the first argument, or use a default if not provided
export app_cpp="${1:-main.cpp}"

# Change to the build directory
cd build || { echo "Failed to cd into build directory"; exit 1; }

# Run cmake and make
cmake ..
make

cd ..


# to compile, just run 
# ~/documents/cpp/chess$ . make.sh testTintoretto.cpp