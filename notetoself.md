# some title

## foreground sprites understand better

the sprites are saved in the OAM? which is an internal memory inside the PPU
OAM can contain up to 64 sprites, where the information of each sprite is 4 bytes:

byte 0: the y position of the **top** of the sprite
byte 1: tile index number
byte 2: attributes byte, contains some information for rendering
byte 3: the x position of the **left** side of the sprite






























## foreground sprites reference

Sprite data is delayed by one scanline; you must subtract 1 from the sprite's Y coordinate before writing it here (byte 0). Hide a sprite by moving it down offscreen, by writing any values between #$EF-#$FF here. Sprites are never displayed on the first line of the picture, and it is impossible to place a sprite partially off the top of the screen. 

#### byte 1 - tile index number.
##### for 8x8 sprites:
(bit 5 of PPUCTRL is 0) this is the tile number of this sprite within the pattern table selected in bit 3 of PPUCTRL
##### for 8x16 sprites:
(bit 5 of PPUCTRL is 1), the PPU ignores the pattern table selection and selects a pattern table from bit 0 of this number. 