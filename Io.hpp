
#ifndef __IO_H__
#define __IO_H__

#define IE_BIT_VBLANK   0
#define IE_BIT_LCDC     1
#define IE_BIT_TIMER    2
#define IE_BIT_SERIAL   3
#define IE_BIT_JOYP     4

#define IF_BIT_VBLANK   0
#define IF_BIT_LCDC     1

#define INTERRUPTS_ENABLE_REG 0xFFFF

#define P1 					0xFF00
#define SERIAL_TX_DATA		0xFF01
#define SERIAL_IO_CTRL		0xFF02
#define DIV					0xFF04
#define TIMA				0xFF05
#define TMA                 0xFF06
#define TAC					0xFF07
#define IF                  0xFF0F
#define NR_10				0xFF10
#define NR_11				0xFF11
#define NR_12				0xFF12
#define NR_13				0xFF13
#define NR_14				0xFF14
#define NR_21               0xFF16
#define NR_22               0xFF17
#define NR_23               0xFF18
#define NR_24               0xFF19
#define NR_30               0xFF1A
#define NR_31               0xFF1B
#define NR_32               0xFF1C
#define NR_33               0xFF1D
#define NR_34               0xFF1E
#define NR_41               0xFF20
#define NR_42               0xFF21 
#define NR_43               0xFF22
#define NR_44               0xFF23
#define NR_50               0xFF24
#define NR_51               0xFF25
#define NR_52               0xFF26
#define WAVE_RAM            0xFF30
#define LCDC                0xFF40 
#define STAT                0xFF41
#define SCY                 0xFF42 
#define SCX                 0xFF43
#define LY                  0xFF44
#define LYC                 0xFF45
#define DMA                 0xFF46
#define BGP                 0xFF47
#define OBJ_PAL_0           0xFF48
#define OBJ_PAL_1           0xFF49
#define WY                  0xFF4A
#define WX                  0xFF4B

#endif


















