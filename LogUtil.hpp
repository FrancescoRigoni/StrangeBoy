
#ifndef __LOGUTIL_H__
#define __LOGUTIL_H__

#include <iostream>
#include <iomanip>
using namespace std;

#define cout8Signed(value) dec << +(int8_t)value
#define cout8Hex(value) hex << "$" << setfill('0') << setw(2) << +value
#define cout16Hex(value) hex << "$" << setfill('0') << setw(4) << value

#define TRACE_CPU_ON

#define OPCODE_PFX     "    : "
#define OPCODE_CB_PFX     " : "

#ifdef TRACE_CPU_ON
    #define TRACE_CPU(expr) cout << expr;
#else
    #define TRACE_CPU(expr) {}
#endif

#endif