StrangeBoy
====
A Game Boy emulator. Work in progress, it runs most games and demos fine.<br>
There are some PPU glitches still to be fixed, especially visible with some demos.<br>

Supported:
* Complete CPU
* PPU
* Timer
* Sound
* Dma
* Div Register
* Timer
* Fake serial unit
* Interrupts VBlank, LCDC, Joypad
* MBC1, including save games

Not supported (yet):
* All other MBCs

![Super Mario Land 3 - Warioland](https://github.com/FrancescoRigoni/GameBoyEmulator/blob/master/screenshots/Strangeboy%20running%20Super%20Mario%20Land%203%20-%20Warioland.png "Super Mario Land 3 - Warioland")

![Tetris](https://github.com/FrancescoRigoni/GameBoyEmulator/blob/master/screenshots/Strangeboy%20running%20Tetris.png "Tetris")

TODO
====
* Code is a bit messy in some places
* MBC1 was quickly implemented and can use improvements
* Some PPU glitches that have to do with the window

Build
=====
Builds on MAC (that's what I use) and Linux (or so I'm told). You need SDL2.<br>
`$ cmake .`<br>
`$ make`<br>
`$ ./StrangeBoy <rom>`<br>

Install SDL2 on Mac
=====
It is needed in order to build the project.
Run in Terminal:
`ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null`
Then
`brew install sdl2`
