#!/bin/bash

if [[ $(pwd) != "/home/roeet/Projects/nes-emulator" ]]
then
    echo "Please run this script from the root directory of the project"
    exit 1
fi

echo "Building emulator..."
g++ -g -c src/cpu.cpp -o cpu.o
g++ -g -c src/operations.cpp -o operations.o
g++ -g -c src/main.cpp -o main.o
g++ -g -c src/bus.cpp -o bus.o
g++ -g -c src/utils.cpp -o util.o

g++ *.o -o emulator 
rm *.o
echo "Done"