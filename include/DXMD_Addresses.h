// Author: Sean Pesce

#pragma once

#ifndef SP_DXMD_MEMORY_ADDRESSES_H_
#define SP_DXMD_MEMORY_ADDRESSES_H_

#include <cstdint>

extern "C" void* prehook_inject_addr = (void*)0x14805e513;  // AoB: 40 8A F3 66 FF CE 50 0F B7 C4 40 FE C6 41 57 41 56 66 40 0F B6 C5
extern "C" void* prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);

extern "C" void* textlist_installer_func = (void*)0x1436A39C0;  // AoB (Steam; after deobfuscation; 2 results): 48 89 5C 24 08 57 48 83 EC 20 48 89 D7 E8 ?? ?? ?? ?? 45 31 C0 41 8D 50 30 48 89 C1 E8 ?? ?? ?? ?? 48 85 C0 74 10 48 89 FA 48 89 C1 E8 ?? ?? ?? ?? 48 89 C3 EB 02
                                                                //                             GoG (5 results): 48 89 5c 24 08 57 48 83 ec 20 48 8b fa e8 ?? ?? ?? ?? 45 33 c0 41 8d 50 ?? 48 8b c8 e8 ?? ?? ?? ?? 48 85 c0 74 10 48 8b d7 48 8b c8 e8 ?? ?? ?? ?? 48 8b d8 eb 02 33 db 48 8b 17 48 89 54 24 38 48 85 d2 74 0a 48 8d 4c 24 38 e8 ?? ?? ?? ?? 48 8d 4c 24 38 48 8b d3 e8 ?? ?? ?? ?? 48 8b 54 24 38 48 85 d2 74 0a 48 8d 4c 24 38 e8 ?? ?? ?? ?? b8 01 00 00 00 48 8b 5c 24 30 48 83 c4 20 5f c3 
extern "C" void* textlist_installer_hook_ret = NULL;

extern "C" void* get_mem_mgr_func = (void*)0x143159300;  // AoB (after deobfuscation): 48 83 EC 28 8B 05 ?? ?? ?? ?? A8 01 75 1E 83 C8 01


void* textlist_str_alloc_call_instruction = (void*)0x1436A101A;  // AoB (Steam; after deobfuscation): E8 ?? ?? ?? ?? 4C 8D 44 24 30 48 8D 54 24 20 48 89 C1 E8 ?? ?? ?? ?? 48 8D 54 24 50
                                                        //                              GoG: e8 ?? ?? ?? ?? 4c 8d 44 24 30 48 8d 54 24 20 48 8b c8 e8 ?? ?? ?? ?? 48 8d 54 24 50 
extern "C" void* textlist_str_alloc_func = (void*)0x14314C0C0;  // AoB (after deobfuscation): 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 89 D3 49 8D 50 01
extern "C" void* textlist_str_alloc_hook_ret = NULL;


extern "C" void* loadingscreen_startsubs_func = (void*)0x1442DB5D0;  // AoB (after deobfuscation): 48 89 5c 24 08 48 89 74 24 10 57 48 83 ec ?? 48 ?? ?? 48 ?? ?? ?? ?? e8
extern "C" void* loadingscreen_video_id_hook_ret = NULL;
extern "C" void* uielement_playvid_func = (void*)0x144859070;  // AoB (after deobfuscation): 48 89 5c 24 10 48 89 74 24 18 57 48 83 ec 70 48 ?? ?? ?? ?? ?? ?? bb 
extern "C" void* uielement_video_id_hook_ret = NULL;


#endif // SP_DXMD_MEMORY_ADDRESSES_H_
