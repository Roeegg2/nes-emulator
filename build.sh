#!/bin/bash

#i know this isnt the most efficient build script, and i should check if the object files are up to date before recompiling each of them
#but it doesnt really matter right now, since the project is so small
#ill transition to cmake when the project gets bigger

if [[ $(pwd) != "/home/roeet/Projects/nes-emulator" ]]
then
    echo "Please run this script from the root directory of the project"
    exit 1
fi

echo "Building emulator..."

g++ -g -c src/cpu.cpp -o cpu.o
g++ -g -c src/ppu.cpp -o ppu.o
g++ -g -c src/operations.cpp -o operations.o
g++ -g -c src/bus.cpp -o bus.o
g++ -g -c src/utils.cpp -o util.o
g++ -g -c src/mapper_n_cart.cpp -o mapper_n_cart.o
g++ -g -c src/main.cpp -o main.o

g++ -g -c src/mappers/nrom_0.cpp -o nrom_0.o

g++ *.o -o emulator 
rm *.o

echo "Done"