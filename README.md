StrangeBoy
====
A Game Boy emulator. Work in progress, it runs most games fine but there is still a lot to do.<br>
There are some PPU glitches still to be fixed.<br>

Supported:
* Complete CPU
* PPU
* Dma
* Div Register
* Interrupts VBlank, LCDC, Joypad
* MBC1 partially, does not save ram banks

Not supported (yet):
* All other MBCs
* Timer
* Sound

![Super Mario Land 3 - Warioland](https://github.com/FrancescoRigoni/GameBoyEmulator/blob/master/screenshots/Strangeboy%20running%20Super%20Mario%20Land%203%20-%20Warioland.png "Super Mario Land 3 - Warioland")

![Tetris](https://github.com/FrancescoRigoni/GameBoyEmulator/blob/master/screenshots/Strangeboy%20running%20Tetris.png "Tetris")


Install SDL2 on Mac
=====
It is needed in order to build the project.
Run in Terminal:
`ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null`
Then
`brew install sdl2`
