# defining compiler and the flags to be used
CXX := g++
CXXFLAGS := -g

# source files
SRCS := cpu.cpp \
	ppu.cpp \
	nes_screen.cpp \
	operations.cpp \
	main.cpp \
	bus.cpp \
	utils.cpp \
	mapper_n_cart.cpp \
	mappers/nrom_0.cpp

# object files
OBJ_DIR := bin
OBJS := $(patsubst %.cpp,bin/%.o,$(SRCS))
# libraries
LIBS := -lSDL2

.PHONY: clean

emulator: setup $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o emulator $(LIBS)

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf bin emulator

setup:
	mkdir -p bin
	mkdir -p bin/mappers