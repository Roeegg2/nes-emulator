# title
feb 18 03:14
okay, so youve got back to working on the emulator.
i reckon its hard going back into the code after such a long time of not touching it, this file is written to help you with that.

last time i just finished "implementing" sprites (meaning i think i did everything correctly, but apperantly i forgot some things or implemented some things wrongly)
like everything should be there, except small details i could have forgot. like the base logic and skeleton should be already here, i reckon just some little stuff i miss understood/didnt implement

if you run the emulator, you see bg is perfectly fine, but you have these strips which are caused by the foreground graphics. this could be a clue there is an issue with the fetching, since its drawing wrong data.
i would maybe start by making sure you fetch everything alright from primary OAM, write things alright to secondary OAM, etc. read the wiki carefully and make sure your code matches exactly what is written there.
you can also ask for help in the nes discord server

another thing that is weird is that they are across the whole screen.
maybe youre fetching the wrong pt byte? maybe its also something to do with the scroll y, maybe thats the problem?

also the frequency of the stripes overlapping the background is very high, so that probably means it has something wrong with its at_byte (byte 2), so again another clue that the problem is with the fetching or something similar.
