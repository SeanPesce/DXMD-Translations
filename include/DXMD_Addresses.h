// Author: Sean Pesce

#pragma once

#ifndef SP_DXMD_MEMORY_ADDRESSES_H_
#define SP_DXMD_MEMORY_ADDRESSES_H_

#include <cstdint>

extern "C" void* prehook_inject_addr = (void*)0x14805e513;  // AoB: 40 8A F3 66 FF CE 50 0F B7 C4 40 FE C6 41 57 41 56 66 40 0F B6 C5
extern "C" void* prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);

extern "C" void* textlist_installer_func = (void*)0x1436A39C0;  // AoB (Steam; after deobfuscation; 2 results): 48 89 5C 24 08 57 48 83 EC 20 48 89 D7 E8 ?? ?? ?? ?? 45 31 C0 41 8D 50 30 48 89 C1 E8 ?? ?? ?? ?? 48 85 C0 74 10 48 89 FA 48 89 C1 E8 ?? ?? ?? ?? 48 89 C3 EB 02
                                                                //             GoG/Breach (5 results, 3rd one): 48 89 5c 24 08 57 48 83 ec 20 48 8b fa e8 ?? ?? ?? ?? 45 33 c0 41 8d 50 ?? 48 8b c8 e8 ?? ?? ?? ?? 48 85 c0 74 10 48 8b d7 48 8b c8 e8 ?? ?? ?? ?? 48 8b d8 eb 02 33 db 48 8b 17 48 89 54 24 38 48 85 d2 74 0a 48 8d 4c 24 38 e8 ?? ?? ?? ?? 48 8d 4c 24 38 48 8b d3 e8 ?? ?? ?? ?? 48 8b 54 24 38 48 85 d2 74 0a 48 8d 4c 24 38 e8 ?? ?? ?? ?? b8 01 00 00 00 48 8b 5c 24 30 48 83 c4 20 5f c3
extern "C" void* textlist_installer_hook_ret = NULL;

extern "C" void* get_mem_mgr_func = (void*)0x143159300;  // AoB (Steam; after deobfuscation): 48 83 EC 28 8B 05 ?? ?? ?? ?? A8 01 75 1E 83 C8 01
                                                         //                       GoG/Breach: 48 83 EC 28 8B 05 ?? ?? ?? ?? A8 01 75 1A 83 C8 01


void* textlist_str_alloc_call_instruction = (void*)0x1436A101A;  // AoB (Steam; after deobfuscation): E8 ?? ?? ?? ?? 4C 8D 44 24 30 48 8D 54 24 20 48 89 C1 E8 ?? ?? ?? ?? 48 8D 54 24 50
                                                                 //                       GoG/Breach: e8 ?? ?? ?? ?? 4c 8d 44 24 30 48 8d 54 24 20 48 8b c8 e8 ?? ?? ?? ?? 48 8d 54 24 50 
extern "C" void* textlist_str_alloc_func = (void*)0x14314C0C0;  // AoB (Steam; after deobfuscation): 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 89 D3 49 8D 50 01
                                                                //                       GoG/Breach: 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D 05 ?? ?? ?? ?? 48 8B DA 49 8D 50 01 48 89 01 49 8B F0 48 8B F9
extern "C" void* textlist_str_alloc_hook_ret = NULL;


extern "C" void* loadingscreen_startsubs_func = (void*)0x1442DB5D0;  // AoB (Steam; after deobfuscation): 48 89 5c 24 08 48 89 74 24 10 57 48 83 ec ?? 48 ?? ?? 48 ?? ?? ?? ?? e8
                                                                     //                       GoG/Breach: 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 60 48 8B F1 48 8D 4C 24 20
extern "C" void* loadingscreen_video_id_hook_ret = NULL;
extern "C" void* loadingscreen_startsubs_get_subs_data_instr = (void*)0x1442DB67B;  // AoB (Steam; after deobfuscation):
                                                                                    //                       GoG/Breach: 48 8D 57 08 48 8D 4C 24 20 E8 ?? ?? ?? ?? 0F57 C0 4C 8D 4C 24 30 48 8D 54 24 20 48 8D 4C 24 40 45 33 C0 0F 29 44 24 30
extern "C" void* loadingscreen_subs_data_hook_ret = NULL;
extern "C" void* str_eq_operator_func = (void*)0x143148340;  // Called a few instructions after loadingscreen_startsubs_get_subs_data_instr
extern "C" void* uielement_playvid_func = (void*)0x144859070;  // AoB (after deobfuscation): 48 89 5c 24 10 48 89 74 24 18 57 48 83 ec 70 48 ?? ?? ?? ?? ?? ?? bb
extern "C" void* uielement_video_id_hook_ret = NULL;
extern "C" void* uicredits_playvid_func = (void*)0x144858FA0;  //  AoB (Steam; after deobfuscation): (Function before uielement_playvid_func)
                                                               // GoG/Breach (2 results; first one): 48 89 5C 24 08 57 48 83 EC 20 48 8B F9 48 8B 49 28 BB 04 00 00 00 48 8B C1 8B D3 0F 1F 44 00 00 0F18 08 48 8D 40 40 48 FF CA
extern "C" void* uicredits_video_id_hook_ret = NULL;
extern "C" void* submgr_startsubs_get_subs_data_instr = (void*)0x1436A5B73;  // AoB (after deobfuscation): 48 8b 45 97 48 8d 4e 10 48 89 46 08 e8
extern "C" void* submgr_startsubs_data_hook_ret = NULL;

extern "C" void* vidscreen_init_func = (void*)0x14475C1F0;  // AoB (Steam, after deobfuscation):
                                                            //                       GoG/Breach: 48 89 5C 24 08 57 48 83 EC 50 48 8B F9 E8 ?? ?? ?? ?? 48 8B 8F 58 01 00 00 BB 04 00 00 00 48 8B C1 8B D3
extern "C" void* vidscreen_init_hook_ret = NULL;
extern "C" void* menuscreen_init_func = (void*)0x143B3B540;  // Called a few instructions after vidscreen_init_func

extern "C" void* renderplayer_start_hook_addr = (void*)0x143615ECC;  // AoB (after deobfuscation): 48 ?? ?? 0F 29 45 E0 FF 50 28 48 89 75 D0
extern "C" void* renderplayer_video_id_hook_ret = NULL;


extern "C" void* ui_font_addr_hook_addr = (void*)0x14367ED20;  // AoB (after deobfuscation): 48 ?? ?? 53 41 56 48 83 EC 58 48 83 79 18 00
extern "C" void* ui_font_addr_hook_ret = NULL;
extern "C" void* ui_font_replace_hook_addr = (void*)0x14494834D;  // AoB (after deobfuscation): ?? ?? ?? 48 ?? ?? ff 50 08 48 ?? ?? 48 85 c0 74 ?? 4c 8b 08
extern "C" void* ui_font_replace_hook_ret = NULL;


extern "C" void* resid_record_mapping_func = (void*)0x143A37490;  // AoB: 48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 ec 70 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? e8
extern "C" void* resid_record_mapping_hook_ret = NULL;


#endif // SP_DXMD_MEMORY_ADDRESSES_H_
