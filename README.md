# nes-emulator

a simple NES emulator written in C++, _currently_ for Linux systems.
Might work on other operating systems, haven't checked.

Currently supported mappers:

 - [x] NROM (Mapper 0)
 - [x] MMC1 (Mapper 1)
 - [x] MMC3 (Mapper 4) ** still haven't implemented the scanline counter
 - [x] CNROM (Mapper 3)
 - [x] UNROM (Mapper 2)


## Before building

1. If you are planning on using a controller, you might have to change the numbers checked on `process_joypad_pressed_buttons()` and `process_joypad_released_buttons()`. To do this add the following for into the source code, and press the controller buttons to see to which numbers they are mapped.

```cpp
for (int i = 0; i < SDL_JoystickNumButtons(joystick); i++) {
    printf("Button %d: %s\n", i, SDL_JoystickGetButton(joystick, i) ? "pressed" : "not pressed");
}
```
2. You can compile with 2 different options:
    a. Debugging mode (uncomment `-DDEBUG` on the `makefile`) (emulator will run slow but output detailed logs)
    b. Release mode - regular compilation (I did not enable aggressive optimizations though. If you want to compile with optimizations, add it manually)

## Building

clone the repo.
make sure you have `make`, `SDL2`,`SDL2 dev package` and `g++` on your system.
then run:

```bash
$ pwd # making sure your on the root directory of the project. make sure `/nes-emulator` is the last directory 
blah/blahblah/.../nes-emulator
$ make
$ ./emulator
```

## Contributing

I would really love someone to develop my emulator! This is just a project I had put on the back burner.
If someone wants to contribute (finish APU, implement more mappers, add proper debugger or whatever really) it would be very cool, and I would be more than happy to work on it together!
My contacts is on my profile 

## Notes

This is not a 100% accurate emulation, there are some things not fully implemented, and some things implemented differently for convenience and simplicity:

- Instead of having 2, 1 bit latches for the lsbit and msbit attribute shift regs and having the shift regs be 8 bits each, i used 8 bit latches and 16 bits shift regs (exactly like the pattern table).
this is for simplicity, and being able to use the same mechanism used for the pattern table shift regs with the attribute shift regs.

- Sprite overflow flag isn't implemented yet, will add support for it in the future though

- Haven't done sound yet, might add it later.

## Final words

If you are interested in emulation development of the NES, go check out my [NES internals blog!](https://roeegg2.github.io) I give try to give a there a simple introduction to it's architecture (at behavior level - I do not go in depth into the specific electronics).
