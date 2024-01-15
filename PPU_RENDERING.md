# PPU RENDERING:

The PPU has the following registers:

v - the current VRAM address
t - temporary VRAM address
w - first/second write toggle register
x - fine x scroll

bg_shift1, bg_shift2 - registers that hold the palette table data
bg_attr1, bg_attr2 - registers that hold the attribute table data


every cycle a bit is fetched from the bg registers and the ppu renders a pixel   
