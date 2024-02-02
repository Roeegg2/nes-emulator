# defining compiler and the flags to be used
CXX := g++
CXXFLAGS := -g -Wall -pedantic

# emulator source files
EMU_SRCS := cpu.cpp \
	ppu.cpp \
	nes_screen.cpp \
	operations.cpp \
	main.cpp \
	bus.cpp \
	utils.cpp \
	mapper_n_cart.cpp \
	mappers/nrom_0.cpp \

EMU_OBJS := $(patsubst %.cpp,bin/%.o,$(EMU_SRCS))

# libraries
EMU_LIBS := -lSDL2

.PHONY: clean


# ... emulator building ...
emulator: setup $(EMU_OBJS) bin/debugger_main.o
	$(CXX) $(CXXFLAGS) $(EMU_OBJS) bin/debugger_main.o -o emulator $(EMU_LIBS)  

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ... debugger building ...
bin/debugger_main.o: debugger/debugger_main.cpp
	$(CXX) $(CXXFLAGS) -c debugger/debugger_main.cpp -o bin/debugger_main.o

setup:
	mkdir -p bin
	mkdir -p bin/mappers

clean:
	rm -rf bin emulator