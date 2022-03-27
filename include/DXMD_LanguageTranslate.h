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

extern "C" uint64_t resource_id = NULL;
extern "C" uint32_t string_id = NULL;
uint8_t string_buf[1048576];  // 1MB
extern "C" void* string_buf_ptr = &string_buf;
extern "C" uint64_t string_len = NULL;
extern "C" void* resource_reader = NULL;  // Temporary storage for pointer to TextList resource reader


std::string calculate_file_md5(std::string& fpath, size_t read_sz = 1048576 /* 1MB */);

#endif // SP_DXMD_LANGUAGE_TRANSLATE_H_
