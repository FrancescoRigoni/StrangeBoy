
#ifndef __LOGUTIL_H__
#define __LOGUTIL_H__

#include <iostream>
#include <iomanip>
using namespace std;

#define cout8Signed(value) dec << +(int8_t)value
#define cout8Hex(value) hex << "$" << setfill('0') << setw(2) << +value
#define cout16Hex(value) hex << "$" << setfill('0') << setw(4) << value

//#define TRACE_CPU_ON
//#define TRACE_STACK_OP_ON
//#define TRACE_OAM_ON
//#define TRACE_VRAM_ON
//#define TRACE_IO_ON
//#define TRACE_PPU_ON
//#define TRACE_JOYPAD_ON
//#define TRACE_DMA_ON
//#define TRACE_SCREEN_ON
//#define TRACE_CART_INFO_ON
//#define TRACE_MBC_ON
//#define TRACE_SOUND_ON

#define OPCODE_PFX     "    : "
#define OPCODE_CB_PFX     " : "

//if (regPC >= 0x017e && regPC <= 0x020b) cout << expr;
#ifdef TRACE_CPU_ON
    #define TRACE_CPU(expr) {                          \
        if (!memory->bootRomEnabled()) cout << expr;   \
    }
#else
    #define TRACE_CPU(expr) {}
#endif

#ifdef TRACE_PPU_ON
    #define TRACE_PPU(expr) cout << expr;
#else
    #define TRACE_PPU(expr) {}
#endif

#ifdef TRACE_STACK_OP_ON
    #define TRACE_STACK_OP(expr) cout << expr;
#else
    #define TRACE_STACK_OP(expr) {}
#endif

#ifdef TRACE_OAM_ON
    #define TRACE_OAM(expr) cout << expr;
#else
    #define TRACE_OAM(expr) {}
#endif

#ifdef TRACE_VRAM_ON
    #define TRACE_VRAM(expr) cout << expr;
#else
    #define TRACE_VRAM(expr) {}
#endif

#ifdef TRACE_IO_ON
    #define TRACE_IO(expr) cout << expr;
#else
    #define TRACE_IO(expr) {}
#endif

#ifdef TRACE_JOYPAD_ON
    #define TRACE_JOYPAD(expr) cout << expr;
#else
    #define TRACE_JOYPAD(expr) {}
#endif

#ifdef TRACE_DMA_ON
    #define TRACE_DMA(expr) cout << expr;
#else
    #define TRACE_DMA(expr) {}
#endif

#ifdef TRACE_SCREEN_ON
    #define TRACE_SCREEN(expr) cout << expr;
#else
    #define TRACE_SCREEN(expr) {}
#endif

#ifdef TRACE_CART_INFO_ON
    #define TRACE_CART_INFO(expr) cout << expr;
#else
    #define TRACE_CART_INFO(expr) {}
#endif

#ifdef TRACE_MBC_ON
    #define TRACE_MBC(expr) cout << expr;
#else
    #define TRACE_MBC(expr) {}
#endif

#ifdef TRACE_SOUND_ON
    #define TRACE_SOUND(expr) cout << expr;
#else
    #define TRACE_SOUND(expr) {}
#endif

#endif