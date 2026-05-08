#!/bin/bash
# Builds the project and creates compile commands for clangd.
mkdir -p build
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
if [ -e build.ninja ]
then
    ninja
else
    make
fi
cp compile_commands.json ..
cd ..
