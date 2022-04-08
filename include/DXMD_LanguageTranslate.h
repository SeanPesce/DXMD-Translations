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


void init_settings();
void init_modifiers();

extern "C" uint8_t translations_enabled = 1;

extern "C" uint64_t textlist_res_id = NULL;
extern "C" uint32_t textlist_str_id = NULL;
extern "C" uint64_t textlist_str_len = NULL;
uint8_t textlist_str_buf[1048576];  // 1MB
extern "C" void* textlist_str_buf_ptr = &textlist_str_buf;
extern "C" void* textlist_res_reader = NULL;  // Temporary storage for pointer to TextList resource reader


std::string calculate_file_md5(std::string& fpath, size_t read_sz = 1048576 /* 1MB */);

#endif // SP_DXMD_LANGUAGE_TRANSLATE_H_
