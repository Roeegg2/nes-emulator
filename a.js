let tn = this.sprite_pattern_shifters[this.secondary_OAM_sprite_index]; // tile number
let sy = this.sprite_y_lines[this.secondary_OAM_sprite_index]; // sprite y
let table = this.io.sprite_pattern_table; // this is a PPU status bit
let attr = this.sprite_attribute_latches[this.secondary_OAM_sprite_index]; // attribute byte
// Vertical flip....
if (attr & 0x80) sy = (this.status.sprite_height - 1) - sy;  // this.status.sprite_height is either 8 or 16 here
if (this.status.sprite_height === 16) {  // if it's an 8x16...
    table = tn & 1; // set the table # to the LSB of the tile number
    tn &= 0xFE; // we don't use the LSB of the tile number in 8x16. top sprite is even
}
if (sy > 7) { // this only happens when we're in the second part of an 8x16 sprite
    sy -= 8;   // this is just to make it fetch the correct line from the 8x8 data
    tn += 1;    // bottom sprite is the top one + 1
}
this.sprite_pattern_shifters[this.secondary_OAM_sprite_index] = this.fetch_chr_line(table, tn, sy);

/**
 * 
 */