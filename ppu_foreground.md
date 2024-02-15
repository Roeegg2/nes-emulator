# PPU foreground

the sprites are saved in the OAM, an internal memory space (ie its not in the cartridge). 
the size of the OAM is `$0100 = 256` bytes. the entry of each sprite here is 4 bytes long. so we get that in total we can store: `256/4 = 64` sprites.
the 4 bytes are used in the following way:

## OAM


### byte 0
this contains the y position of the **top** part of the sprite. this value is given by: _scanline - 1_, (where _scanline_ is the scanline to have the top of the sprite be placed), because sprites cant be placed on the first scanline.

### byte 1
this byte is used to index into the pattern table. 
the behaviour of this byte differs between 8x8 sprites and 8x16 sprites.

#### 8x8:
this byte is combined with bit 3 of PPUCTRL to create an address into the pattern table, where _bit 3 of ppuctrl_ is used to select the pattern table ($0000, or $1000), and the _byte_ is used as an index into that pattern table.

#### 8x16:
_bit 0 of this byte_ is used to select the pattern table, and _the rest of the bits_ (ie 1-7) is used to index into the pattern table.
here is a great diagram taken from the NESdev wiki:

```
76543210
||||||||
|||||||+- Bank ($0000 or $1000) of tiles
+++++++-- Tile number of top of sprite (0 to 254; bottom half gets the next tile)

```

### byte 2
this byte contains a bunch of different information regrading the sprite, used for rendering.
here is another great diagram, taken from the NESdev wiki:

```

76543210
||||||||
||||||++- Palette (4 to 7) of sprite
|||+++--- Unimplemented (read 0)
||+------ Priority (0: in front of background; 1: behind background)
|+------- Flip sprite horizontally
+-------- Flip sprite vertically

```

- *palette -* 2 bits used to select the palette of the sprite. the same way the 2 bits in the attribute table are used to select the palette group for the background (only this time we select from palettes 4-7 instead of 0-3)

- *unimplemented -* pretty self explanitary, unused and is always 0.

- *priority -* sprites can be rendered both behind, and infront of the background. again, pretty self explanitary.

- *flipping -* ~~complete this~~


### byte 3
this byte dictates the x position of the **left** part of the sprite. 
very similar to byte 0.



## DMA
this is a section in the _cpu ram_ that some program use as a "temporary OAM".
what do i mean?
instead of having the cpu directly write to _OAM_, it writes to _DMA_ and then it copies the data from _DMA_ into _OAM_.
the cpu does that transfer by first writing an _address "N"_ to a register called _OAMDMA_, and then writing each byte of the 256 bytes stored in _DMA_ into another register called _OAMDATA_.
the data in _OAMDATA_ is placed in the address 


(usually resides in $0200-$02FF) 





























## foreground sprites reference

Sprite data is delayed by one scanline; you must subtract 1 from the sprite's Y coordinate before writing it here (byte 0). Hide a sprite by moving it down offscreen, by writing any values between #$EF-#$FF here. Sprites are never displayed on the first line of the picture, and it is impossible to place a sprite partially off the top of the screen. 

#### byte 1 - tile index number.
##### for 8x8 sprites:
(bit 5 of PPUCTRL is 0) this is the tile number of this sprite within the pattern table selected in bit 3 of PPUCTRL
##### for 8x16 sprites:
(bit 5 of PPUCTRL is 1), the PPU ignores the pattern table selection and selects a pattern table from bit 0 of this number. 