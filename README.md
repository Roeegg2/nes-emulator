# nes-emulator

a simple NES emulator written in C++, _currently_ for Linux systems.
Might work on other operating systems, havent checked.

## Before building

1. If you are planning on using a controller, you might have to change the numbers checked on `process_joypad_pressed_buttons(Controller* controller)` and `process_joypad_released_buttons(Controller* controller)`. To do this add the following for into the source code, and press the controller buttons to see to which numbers they are mapped.

```cpp
for (int i = 0; i < SDL_JoystickNumButtons(joystick); i++) {
    printf("Button %d: %s\n", i, SDL_JoystickGetButton(joystick, i) ? "pressed" : "not pressed");
}
```

## Building

make sure you have `make`, `sdl2` and `gcc` on your system.
then run:

```bash
$ pwd # making sure your on the root directory of the project. make sure `/nes-emulator` is the last directory 
blah/blahblah/.../nes-emulator
$ make
$ ./emulator
```

## Notes

This is not a 100% accurate emulation, there are some things not fully implemented, and some things implemented differently for convenience and simplicity:

- Instead of having 2, 1 bit latches for the lsbit and msbit attribute shift regs and having the shift regs be 8 bits each, i used 8 bit latches and 16 bits shift regs (exactly like the pattern table).
this is for simplicity, and being able to use the same mechanism used for the pattern table shift regs with the attribute shift regs.
- Sprite overflow flag isn't implemented yet, will add support for it in the future though

## Final words

If you are interested in emulation development of the NES, go check out my [https://roeegg2.github.io/nes-internals-blog] (NES internals blog)! I give try to give a there a simple introduction to it's architecture (at emulation level, I do not go in depth into the electronics).
