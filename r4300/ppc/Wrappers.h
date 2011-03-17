/**
 * Wii64 - Wrappers.h
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * 
 * Interface between emulator code and recompiled code
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "Recompile.h"
#include "../r4300.h"

#define DYNAREG_REG    14
#define DYNAREG_COP0   15
#define DYNAREG_FPR_32 16
#define DYNAREG_FPR_64 17
#define DYNAREG_FCR31  18
#define DYNAREG_RDRAM  19
#define DYNAREG_LADDR  20
#define DYNAREG_NINTR  21
#define DYNAREG_FUNC   22
#define DYNAREG_ZERO   23

#define DYNAOFF_LR     20

#define REG_LOCALRS    34

extern int noCheckInterrupt;

typedef enum { MEM_LW,   MEM_LH,   MEM_LB,   MEM_LD,
               MEM_LWU,  MEM_LHU,  MEM_LBU,
               MEM_LWC1, MEM_LDC1,
               MEM_SW,   MEM_SH,   MEM_SB,   MEM_SD,
               MEM_SWC1, MEM_SDC1                    } memType;

unsigned int decodeNInterpret(MIPS_instr, unsigned int, int);
#ifdef COMPARE_CORE
int dyna_update_count(unsigned int pc, int isDelaySlot);
#else
int dyna_update_count(unsigned int pc);
#endif
unsigned int dyna_check_cop1_unusable(unsigned int pc, int isDelaySlot);
unsigned int dyna_mem(unsigned int value, unsigned int addr,
                      memType type, unsigned int pc, int isDelaySlot);

//cop0 macros
#define Index r4300.reg_cop0[0]
#define Random r4300.reg_cop0[1]
#define EntryLo0 r4300.reg_cop0[2]
#define EntryLo1 r4300.reg_cop0[3]
#define Context r4300.reg_cop0[4]
#define PageMask r4300.reg_cop0[5]
#define Wired r4300.reg_cop0[6]
#define BadVAddr r4300.reg_cop0[8]
#define Count r4300.reg_cop0[9]
#define EntryHi r4300.reg_cop0[10]
#define Compare r4300.reg_cop0[11]
#define Status r4300.reg_cop0[12]
#define Cause r4300.reg_cop0[13]
#define EPC r4300.reg_cop0[14]
#define PRevID r4300.reg_cop0[15]
#define Config r4300.reg_cop0[16]
#define LLAddr r4300.reg_cop0[17]
#define WatchLo r4300.reg_cop0[18]
#define WatchHi r4300.reg_cop0[19]
#define XContext r4300.reg_cop0[20]
#define PErr r4300.reg_cop0[26]
#define CacheErr r4300.reg_cop0[27]
#define TagLo r4300.reg_cop0[28]
#define TagHi r4300.reg_cop0[29]
#define ErrorEPC r4300.reg_cop0[30]

#endif

