# PPU Graphics explained:

## Terminology:

- Tiles - 8x8 pixel regions, which the nes screen is divided into
- Sprites - any shape to be drawn on the screen. Mario, Goomba, clouds, Grass, Sky - everything.
- Character Sprites - a subset of the Sprites. Stuff like Mario, Goomba, Fireball, Koopatroopa, etc (ie all sprites which are not background)

## Nametables:
Nametables can be thought of as "location of each sprite on the screen" I know its a bit confusing, hopefully the following explanation clears some of the confusion.

the PPU has 4 nametables, each of size 1024 bytes (see PPU memory map). 
Each nametable contains:
- a 30x32 table, each entry being a byte (so 960 bytes total)
- a 2x32 table, each entry being a byte, called the attribute table (so 64 bytes total), which is used as part of the color selecting mechanism (ill explain that more in detail later)

so together we have 1024 bytes.
and remember, we have 4 of these - so total vram is 4 * 1024 = 4096 bytes (the nametables are stored in *vram*)

but what does each entry contain?

glad you asked!

each entry is an index into the pattern table. What do i mean?


what is the pattern table?

glad you asked!

## Pattern table:
the pattern table is another section, its usually rom (but can be ram as well). it is in the *cartridge*. also called sometimes CHR rom/ram.
remember i said the nametables are the "shape of the whole screen"? well, the pattern table can be thought of the "shape of each sprite"

the NES has a minimum of 2 pattern tables ($0000-$0FFF for Pattern table 0, $1000-$1FFF for Pattern table 1). It can have more, but that depends on the mapper.

each pattern table's size is $1000 (4096) bytes.

each tile in the pattern table is represented in a unique way, using 2 planes.
each plane is 8 bytes, so 16 bytes total for each tile.

one plane is used to specify the lsb, the other is for the msb.
together they give a 2bit value to each pixel.

An example taken from the NESdev wiki:
(trying to draw a '½' symbol on the screen)

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

