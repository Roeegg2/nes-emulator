# APU

## Terminology:
- Sweeping - the changing of the frequency in real time
- Length - how long the audio is playing

the apu has 5 audio channels:
pulse wave 0
pulse wave 1
triangle wave
noise
DMC

pulse wave 0, pulse wave 1, triangle wave, and noise each have 4 registers devoted to them. Here's the memory layout (from the CPU's perspective of course, since it is the one controlling communicating with the APU)

$4000-$4003 - pulse wave 0
$4004-$4007 - pulse wave 1
$4008-$400B - triangle wave
$400C-$400F - noise

