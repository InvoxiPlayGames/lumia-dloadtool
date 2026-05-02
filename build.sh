#!/bin/bash
# Generates compile_commands.json for clangd
mkdir -p build
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make
cp compile_commands.json ..
cd ..
