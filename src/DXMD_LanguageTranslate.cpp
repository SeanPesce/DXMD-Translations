/*
    CONTRIBUTORS:
        Sean Pesce

*/

#include <inttypes.h>
#include <fstream>
#include <map>

#include "sp/memory.h"
#include "sp/memory/injection/asm/x64.h"

#include "DXMD_LanguageTranslate.h"

#include <Windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include <Objbase.h>
#pragma  comment(lib, "dbghelp")
#pragma  comment(lib, "psapi")


extern sp::io::ps_ostream debug;

extern "C" uint64_t resource_id;
extern "C" uint32_t string_id;
extern uint8_t string_buf[1048576];
extern "C" void* string_buf_ptr;
extern "C" uint64_t string_len;


std::map<uint64_t, std::map<uint32_t, std::string>> translation_data;


extern "C" void hook_installer();
extern "C" void textlist_installer_hook();
extern "C" void str_alloc_hook();


// This function was developed using code from Franc[e]sco (from ccplz.net)
BOOL get_module_size(HMODULE hmodule, LPVOID* lplp_base, PDWORD64 lpdw_size)
{
    if (hmodule == GetModuleHandle(NULL)) {
        PIMAGE_NT_HEADERS pImageNtHeaders = ImageNtHeader((PVOID)hmodule);

        if (pImageNtHeaders == NULL)
            return FALSE;

        *lplp_base = (LPVOID)hmodule;
        *lpdw_size = pImageNtHeaders->OptionalHeader.SizeOfImage;
    }
    else {
        MODULEINFO  ModuleInfo;

        if (!GetModuleInformation(GetCurrentProcess(), hmodule, &ModuleInfo, sizeof(MODULEINFO)))
            return FALSE;

        *lplp_base = ModuleInfo.lpBaseOfDll;
        *lpdw_size = ModuleInfo.SizeOfImage;
    }
    return TRUE;
}


void load_translation_json(const char* fpath)
{
    if (!fpath || !strnlen_s(fpath, MAX_PATH))
    {
        debug.print("No translation file specified.\n");
        return;
    }
    // Load JSON data
    debug.print("Loading strings from " + std::string(fpath) + "...\n");
    std::ifstream istream(fpath);
    nlohmann::json json;
    bool parse_failed = false;
    try
    {
        istream >> json;
    }
    catch (nlohmann::json::parse_error& ex)
    {
        parse_failed = true;
        std::string msg = "Error parsing JSON at byte " + std::to_string(ex.byte) + "\n";
        if (ex.byte == 1)
        {
            msg = "Error parsing JSON. File path may be incorrect.";
        }
        debug.print(msg);
        MessageBox(NULL, ("Failed to parse " + (fpath + ("\n\n" + msg))).c_str(), "ERROR", MB_OK);
    }
    istream.close();
    if (parse_failed)
    {
        return;
    }

    sp::io::ps_ostream progress_display("Loading...");
    progress_display.start();
    progress_display.print("+----------------------------+\r\n|    DXMD Translation Mod    |\r\n|     Author: Sean Pesce     |\r\n+----------------------------+\r\nCompiled: " __DATE__ "  " __TIME__ "\r\n\r\n");
    progress_display.print("Loading " + json["language_long"].get<std::string>() + " translation data from " + std::string(fpath) + "\n");

    // Convert JSON data to C++ map
    size_t file_count = json["files"].size();
    for (size_t i = 0; i < file_count; i++)
    {
        auto f = json["files"][i];
        size_t str_count = f["content"].size();
        //debug.print("Parsing resource: " + f["resource"].get<std::string>() + "\n");
        progress_display.print("Progress: " + std::to_string(i) + "/" + std::to_string(file_count) + "\r");
        std::map<uint32_t, std::string> textlist_data;

        for (size_t j = 0; j < str_count; j++)
        {
            auto s = f["content"][j];
            textlist_data.insert({ s["id"].get<uint32_t>(), s["string"].get<std::string>() });
        }

        translation_data.insert({ f["id"].get<uint64_t>(), textlist_data });
    }
    progress_display.terminate();
}
extern "C" void* load_translation_json_ptr = &load_translation_json;


// Returns 0 if the string was found; returns 1 otherwise
extern "C" int get_translation_string(uint64_t res_id, uint32_t str_id, std::string& out)
{
    //debug.print("Loading string: " + std::to_string(res_id) + "->" + std::to_string(str_id) + "\n");
    if (translation_data.find(res_id) != translation_data.end())
    {
        //debug.print("Found resource ID.\n");
        auto textlist_data = translation_data[res_id];
        if (textlist_data.find(str_id) != textlist_data.end())
        {
            //debug.print("Loading string: " + std::to_string(str_id) + "\n");
            out = textlist_data[str_id];
            return 0;
        }
    }
    // Failed to find the string
    //debug.print("String not found.\n");
    return 1;
}


// Install hooks in TextList loader and language string allocator
extern "C" void install_translation_hooks()
{
    sp::mem::code::x64::inject_jmp_14b(textlist_installer_func, &textlist_installer_hook_ret, 4, textlist_installer_hook);
    sp::mem::code::x64::inject_jmp_14b(str_alloc_call_instruction, &str_alloc_hook_ret, 1, str_alloc_hook);
}
extern "C" void* install_translation_hooks_ptr = &install_translation_hooks;


// Install a pre-hook hook that installs the translation hooks after the target memory is deobfuscated
void install_pre_hook()
{
    if (!prehook_inject_addr)
    {
        // No pre-hook needed; directly install the translation hooks
        install_translation_hooks();
        return;
    }
    // Original bytes at 0x14805e513: 40 8A F3 66 FF CE 50 0F B7 C4 40 FE C6 41 57 41
    sp::mem::set_protection((void*)prehook_inject_addr, 15 + 1, MEM_PROTECT_RWX);
    uint8_t prehook_jmp[15] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x90 };  // jmp hook_installer
    // Insert function address as operand in pre-hook jmp instruction
    *(uint64_t*)(&prehook_jmp[6]) = (uint64_t)(&hook_installer);
    // Write the jmp instruction
    memcpy((void*)prehook_inject_addr, prehook_jmp, 15);
}


// Called from TextList loader hook; stores translated string information for later replacement
extern "C" void store_string_info()
{
    std::string tmp_str;
    int retval = get_translation_string(resource_id, string_id, tmp_str);
    if (retval)
    {
        // String was not found; don't change original string information
        string_buf[0] = 0;
        return;
    }

    // String was found
    string_buf_ptr = &string_buf;
    string_len = tmp_str.size();
    memcpy_s(string_buf, sizeof(string_buf), tmp_str.c_str(), tmp_str.size());

    // Probably not necessary, but null-terminate the string just in case
    if (tmp_str.size() < sizeof(string_buf))
    {
        string_buf[tmp_str.size()] = 0;
    }
    else
    {
        string_buf[sizeof(string_buf) - 1] = 0;
    }
}
extern "C" void* store_string_info_ptr = &store_string_info;


std::string calculate_file_md5(std::string& fpath, size_t read_sz)
{
    uint8_t* buf = (uint8_t*)CoTaskMemAlloc(read_sz);
    if (!buf)
    {
        // Memory allocation failed
        MessageBox(NULL, "Failed to allocate memory for MD5 ingress buffer.", "ERROR", MB_OK);
        ExitProcess(SP_ERR_INSUFFICIENT_BUFFER);
    }

    std::fstream f;
    f.open(fpath, std::ios::in|std::ios::binary);

    MD5 md5;

    while (true)
    {
        f.read((char*)buf, read_sz);
        size_t nbytes = f.gcount();
        if (!nbytes)
        {
            break;
        }

        md5.add((const void*)buf, nbytes);

        if (f.eof())
        {
            break;
        }
    }

    CoTaskMemFree(buf);
    return md5.getHash();
}
