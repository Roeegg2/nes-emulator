# NES PPU memory map:

# Preface:

How is the data to draw stored in the NES? How does the PPU know how to analyze and draw it?
My aim is to give you a very basic yet correct visualization of how the graphics are stored in the NES.
Along side explanations, I will take the game Super Mario Bros 1 as an example to illusrate and show how its actually implemented.

The PPU divides the stuff to draw into 2 categories:
1. Background - As it implies, this is (usually) just the background (could be other stuff as well though. For example, if I remember correctly Donkey Kong is considered part of the background). (Examples for this in Super Mario Bros 1 would be the clouds, the sky, etc)
2. Sprites - Usually stuff like character sprites, special effects, **health/status bars, etc (For example, Mario, Goomba, fireball)

NOTE: this causes a lot of confusion in beguinners - sprites and character sprites are 2 different things. Character sprites are as I explained: Mario, Goomba, Koopa Troopa, etc. Sprites on the other hand are a piece of graphics to draw. I know this might be a bit confusing, It'll hopefully be clearer near the end

# Terminology:
Some of the things here might not be completely clear right away, hopefully things get clearer as we progress.

1. Tile - 8x8 pixel grid

# Pattern table:

What is a pattern table?
A pattern table is where the *shape* of sprites are stored. (This is the CHR data on the cartridge)

The PPU can address 2 pattern tables, which are located on the cartridge. In the PPU's memory map they reside in:
$0000 - $0fff - this is the address range of Pattern Table 1
$1000 - $1fff - this is the address range of Pattern Table 2

Each entry in a pattern table represents the shape of a tile on the screen, in the following way:

The NES has color depth of 2 bits. That means that each color is represented with 2 bits of information. So we have 2^2 = 4 - color option (00, 01, 10, 11). 
Using a specific pattern of colors we can draw sprites.

Each pattern table is splitted into 2 "planes" (sections), where the first plane dictates the msb bit and the second plane dictates the lsb. 
Here's an example taken from the NESdev wiki, where we try to draw a sprite in the shape of a '½' in the size of a tile:

Bit Planes            Pixel Pattern
$0xx0=$41  01000001
$0xx1=$C2  11000010
$0xx2=$44  01000100
$0xx3=$48  01001000
$0xx4=$10  00010000
$0xx5=$20  00100000         .1.....3
$0xx6=$40  01000000         11....3.
$0xx7=$80  10000000  =====  .1...3..
                            .1..3...
$0xx8=$01  00000001  =====  ...3.22.
$0xx9=$02  00000010         ..3....2
$0xxA=$04  00000100         .3....2.
$0xxB=$08  00001000         3....222
$0xxC=$16  00010110
$0xxD=$21  00100001
$0xxE=$42  01000010
$0xxF=$87  10000111

The image *could* look like this: (NOTE: notice the *could* - I will shortly later why is that)
![Alt text](image.png)

Every sprite in the NES is stored that way in one of the pattern tables:


