// Author: Sean Pesce

#pragma once

#ifndef SP_DXMD_MEMORY_ADDRESSES_H_
#define SP_DXMD_MEMORY_ADDRESSES_H_

#include <cstdint>

extern "C" void* prehook_inject_addr = (void*)0x14805e513;  // AoB: 40 8A F3 66 FF CE 50 0F B7 C4 40 FE C6 41 57 41 56 66 40 0F B6 C5
extern "C" void* prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);

extern "C" void* textlist_installer_func = (void*)0x1436A39C0;  // AoB (after deobfuscation; 2 results): 48 89 5C 24 08 57 48 83 EC 20 48 89 D7 E8 ?? ?? ?? ?? 45 31 C0 41 8D 50 30 48 89 C1 E8 ?? ?? ?? ?? 48 85 C0 74 10 48 89 FA 48 89 C1 E8 ?? ?? ?? ?? 48 89 C3 EB 02
extern "C" void* textlist_installer_hook_ret = NULL;

extern "C" void* get_mem_mgr_func = (void*)0x143159300;  // AoB (after deobfuscation): 48 83 EC 28 8B 05 ?? ?? ?? ?? A8 01 75 1E 83 C8 01


void* str_alloc_call_instruction = (void*)0x1436A101A;  // AoB (after deobfuscation): E8 ?? ?? ?? ?? 4C 8D 44 24 30 48 8D 54 24 20 48 89 C1 E8 ?? ?? ?? ?? 48 8D 54 24 50
extern "C" void* str_alloc_func = (void*)0x14314C0C0;  // AoB (after deobfuscation): 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 89 D3 49 8D 50 01
extern "C" void* str_alloc_hook_ret = NULL;


#endif // SP_DXMD_MEMORY_ADDRESSES_H_
