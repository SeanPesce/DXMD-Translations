// Author: Sean Pesce

#pragma once

#ifndef SP_DXMD_LANGUAGE_TRANSLATE_H_
#define SP_DXMD_LANGUAGE_TRANSLATE_H_

#include <Windows.h>
#include <cstdint>

#include "lib/nlohmann_json.hpp"
#include "lib/stbrumme_md5.h"

#include "sp/io/powershell_ostream.h"

#include "DXMD_Addresses.h"


#define _TEXTLIST_STR_BUF_LEN 1048576  // 1MB
#define _SUBTITLES_STR_BUF_LEN _TEXTLIST_STR_BUF_LEN


void init_settings();
void init_modifiers();

extern "C" uint8_t translations_enabled = 1;

extern "C" uint64_t textlist_res_id = NULL;
extern "C" uint32_t textlist_str_id = NULL;
extern "C" uint64_t textlist_str_len = NULL;
uint8_t textlist_str_buf[_TEXTLIST_STR_BUF_LEN];
extern "C" void* textlist_str_buf_ptr = &textlist_str_buf;
extern "C" void* textlist_res_reader = NULL;  // Temporary storage for pointer to TextList resource reader


extern "C" uint64_t video_res_id = NULL;
extern "C" uint8_t translate_next_subtitle = 0;
extern "C" uint8_t playing_video_no_load_screen = 0;  // True if playing a non-loading-screen video
uint8_t subtitles_str_buf[_SUBTITLES_STR_BUF_LEN];
char* subtitles_str_buf_ptr = (char*)&subtitles_str_buf[sizeof(void*)+8];
uint32_t subtitles_capacity = _SUBTITLES_STR_BUF_LEN - 16;
extern "C" void* subtitles_zstr_buf_ptr = &subtitles_str_buf;


std::string calculate_file_md5(std::string& fpath, size_t read_sz = 1048576 /* 1MB */);

#endif // SP_DXMD_LANGUAGE_TRANSLATE_H_
