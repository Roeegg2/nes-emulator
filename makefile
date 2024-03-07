# defining compiler and the flags to be used
CXX := g++
CXXFLAGS := -g -Wall -pedantic -std=c++20 \
# -DDEBUG
# -fsanitize=shift -fsanitize=undefined -fsanitize=address


# emulator source files
EMU_SRCS := cpu.cpp \
	ppu.cpp \
	nes_screen.cpp \
	operations.cpp \
	controller.cpp \
	main.cpp \
	bus.cpp \
	utils.cpp \
	mapper_n_cart.cpp \
	mappers/nrom_0.cpp \
	mappers/cnrom_3.cpp \
	mappers/unrom_2.cpp \
	mappers/mmc1_1.cpp \

EMU_OBJS := $(patsubst %.cpp,bin/%.o,$(EMU_SRCS))

# libraries
EMU_LIBS := -lSDL2

.PHONY: clean

# ... emulator building ...
emulator: setup $(EMU_OBJS)
	$(CXX) $(CXXFLAGS) $(EMU_OBJS) -o emulator $(EMU_LIBS)  

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

setup:
	mkdir -p bin
	mkdir -p bin/mappers

clean:
	rm -rf bin emulator