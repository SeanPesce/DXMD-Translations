// Author: Sean Pesce

#include <Windows.h>

#include <inttypes.h>
#include <filesystem>

#include "lib/nlohmann_json.hpp"

#include "sp/environment.h"
#include "sp/file.h"
#include "sp/io/powershell_ostream.h"

const char *cfg_file = ".\\DXMD_Mod.ini";
sp::io::ps_ostream debug;

// Configurable settings
extern nlohmann::json json;
std::string log_file = ".\\DXMD_Mod.log";
extern "C" uint8_t translations_enabled;

void load_original_dll();
int get_dll_chain();
void init_settings();
HANDLE async_thread_handle = NULL;
DWORD WINAPI async_thread(LPVOID param);

const char *lib_name = "version";
#define DLL_EXPORT_COUNT_ 15
LPCSTR import_names[DLL_EXPORT_COUNT_] = { "GetFileVersionInfoA", "GetFileVersionInfoByHandle", "GetFileVersionInfoExW", "GetFileVersionInfoSizeA", "GetFileVersionInfoSizeExW", "GetFileVersionInfoSizeW", "GetFileVersionInfoW", "VerFindFileA", "VerFindFileW", "VerInstallFileA", "VerInstallFileW", "VerLanguageNameA", "VerLanguageNameW", "VerQueryValueA", "VerQueryValueW" };

HINSTANCE dll_instance = NULL;
HINSTANCE dll_chain_instance = NULL;
extern "C" UINT_PTR export_locs[DLL_EXPORT_COUNT_] = { 0 };

uint64_t dxmd_base = NULL; // DXMD.exe base address
DWORD64 dxmd_size = 0;    // DXMD.exe memory size


extern void install_pre_hook();
extern void load_translation_json(const char* fpath);
extern std::string calculate_file_md5(std::string& fpath, size_t read_sz = 1048576 /* 1MB */);
extern BOOL get_module_size(HMODULE hModule, LPVOID* lplpBase, PDWORD64 lpdwSize);


BOOL WINAPI DllMain(HINSTANCE hinst_dll, DWORD fdw_reason, LPVOID lpv_reserved)
{
    dll_instance = hinst_dll;
    if (fdw_reason == DLL_PROCESS_ATTACH) {
        // Set working directory
        SetCurrentDirectory(sp::env::lib_dir().c_str());

        debug = sp::io::ps_ostream("DXMD Mod Debug");
        if (GetPrivateProfileInt("DLL", "Debug", 0, cfg_file))
        {
            debug.start();
        }

        char cfg_str[MAX_PATH];
        GetPrivateProfileString("DLL", "LogFile", log_file.c_str(), cfg_str, MAX_PATH, cfg_file);
        log_file = cfg_str;
        debug.print("Writing log to " + log_file + "\n");

        debug.print("\n+----------------------------+\r\n|    DXMD Translation Mod    |\r\n|     Author: Sean Pesce     |\r\n+----------------------------+\r\nCompiled: " __DATE__ "  " __TIME__ "\r\n\r\nAttached to process.\n");
        dxmd_base = NULL;
        dxmd_size = 0;
        debug.print("Obtaining module base & size...\n");
        get_module_size(GetModuleHandle(NULL), (LPVOID*)&dxmd_base, &dxmd_size); // Obtain DXMD.exe base address & size
        get_dll_chain();

        debug.print("Finished loading settings.\n");
        if (!dll_chain_instance) {
            // No chain loaded; get original DLL from system directory
            load_original_dll();
        }
        if (!dll_chain_instance) {
            debug.print("Failed to load original DLL. Exiting...\n");
            MessageBox(NULL, ("Failed to load original " + std::string(lib_name) + ".dll").c_str(), "ERROR", MB_OK);
            return FALSE;
        }
        debug.print("Loading exported funcs...\n");
        for (int i = 0; i < DLL_EXPORT_COUNT_; i++) {
            export_locs[i] = (UINT_PTR)GetProcAddress(dll_chain_instance, import_names[i]);
            std::stringstream strstr;
            strstr << std::hex << export_locs[i];
            debug.print("    " + std::string(import_names[i]) + " @ " + strstr.str() + "\n");
        }

        debug.print("Initializing keybinds & additional settings...\n");
        init_settings();
        debug.print("Finished reading keybinds & settings.\n");

        // Initialize thread(s)
        debug.print("Initializing async thread...\n");
        async_thread_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&async_thread, 0, 0, 0);
    } else if (fdw_reason == DLL_PROCESS_DETACH) {
        debug.print("Detaching from process.\n");
        FreeLibrary(dll_chain_instance);
    }
    return TRUE;
}


extern "C" void GetFileVersionInfoA_wrapper();
extern "C" void GetFileVersionInfoByHandle_wrapper();
extern "C" void GetFileVersionInfoExW_wrapper();
extern "C" void GetFileVersionInfoSizeA_wrapper();
extern "C" void GetFileVersionInfoSizeExW_wrapper();
extern "C" void GetFileVersionInfoSizeW_wrapper();
extern "C" void GetFileVersionInfoW_wrapper();
extern "C" void VerFindFileA_wrapper();
extern "C" void VerFindFileW_wrapper();
extern "C" void VerInstallFileA_wrapper();
extern "C" void VerInstallFileW_wrapper();
extern "C" void VerLanguageNameA_wrapper();
extern "C" void VerLanguageNameW_wrapper();
extern "C" void VerQueryValueA_wrapper();
extern "C" void VerQueryValueW_wrapper();


// Loads the original DLL from the default system directory
void load_original_dll()
{
    debug.print("Loading original DLL...\n");
    char buffer[MAX_PATH];

    // Get path to system directory
    debug.print("    Getting system directory...\n");
    GetSystemDirectory(buffer, MAX_PATH);

    // Append DLL name
    strcat_s(buffer, (std::string("\\") + lib_name + ".dll").c_str());
    debug.print(std::string("    Sys dir: ") + buffer + "\n");

    // Try to load the system DLL, if pointer empty
    if (!dll_chain_instance) dll_chain_instance = LoadLibrary(buffer);

    // Debug
    if (!dll_chain_instance)
    {
        debug.print("Failed to load original DLL. Exiting...\n");
        MessageBox(NULL, ("Failed to load original " + std::string(lib_name) + ".dll").c_str(), "ERROR", MB_OK);
        ExitProcess(SP_ERR_FILE_NOT_FOUND); // Exit
    }
    debug.print("    Loaded original DLL.\n");
}


// Parses configuration file for DLL intialization settings
int get_dll_chain()
{
    debug.print("Initializing DLL settings...\n");
    char dll_chain_buffer[MAX_PATH];
    // Check settings file for DLL chain
    debug.print("DLL chain:\n");
    GetPrivateProfileString("DLL", "Chain", NULL, dll_chain_buffer, MAX_PATH, cfg_file);

    if (dll_chain_buffer[0] != '\0')
    {
        // Found DLL_Chain entry in settings file
        debug.print(std::string("    \"") + dll_chain_buffer + std::string("\"\n"));
        dll_chain_instance = LoadLibrary(dll_chain_buffer);
        if (!dll_chain_instance)
        {
            // Failed to load wrapper DLL
            debug.print("Failed to load chain.\n");
            return 2; // Return 2 if given DLL could not be loaded
        }
    }
    else
    {
        debug.print("    No chain specified.\n");
        return 1; // Return 1 if config file or DLL_Chain entry could not be located
    }
    return 0; // Success
}


// Version-specific memory addresses
extern "C" void* prehook_inject_addr;
extern "C" void* prehook_ret;
extern "C" void* textlist_installer_func;
extern "C" void* get_mem_mgr_func;
extern void* textlist_str_alloc_call_instruction;
extern "C" void* textlist_str_alloc_func;
extern "C" void* loadingscreen_startsubs_func;
extern "C" void* loadingscreen_startsubs_get_subs_data_instr;
extern "C" void* str_eq_operator_func;
extern "C" void* uielement_playvid_func;
extern "C" void* submgr_startsubs_get_subs_data_instr;
extern "C" void* vidscreen_init_func;
extern "C" void* menuscreen_init_func;

/**
 *  Determine keybinds and global configuration settings for the plugin
 */
void init_settings()
{

    // Set memory addresses based on game executable MD5 (crowd-sourced hashes)
    std::string exe_md5 = calculate_file_md5(sp::env::exe_path());
    debug.print(sp::env::exe_name() + " MD5: " + exe_md5 + "\n");

    if (exe_md5 == "a227bc2145d592f0f945df9b882f96d8")  // v1.19-801.0 Steam
    {
        prehook_inject_addr = (void*)(dxmd_base + 0x805E513);
        prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);  // DXMD.exe+0x805E522
        get_mem_mgr_func = (void*)(dxmd_base + 0x3159300);

        textlist_installer_func = (void*)(dxmd_base + 0x36A39C0);
        textlist_str_alloc_call_instruction = (void*)(dxmd_base + 0x36A101A);
        textlist_str_alloc_func = (void*)(dxmd_base + 0x314C0C0);

        loadingscreen_startsubs_func = (void*)(dxmd_base + 0x42DB5D0);
        loadingscreen_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0x42DB68A);
        str_eq_operator_func = (void*)(dxmd_base + 0x3148340);
        uielement_playvid_func = (void*)(dxmd_base + 0x4859070);
        submgr_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0x36A5B67);
        vidscreen_init_func = (void*)(dxmd_base + 0x475C1F0);
        menuscreen_init_func = (void*)(dxmd_base + 0x3B3B540);
    }
    else if (exe_md5 == "3745fa30bf3f607a58775f818c5e0ac0"     // v1.19-801.0 GoG
            || exe_md5 == "c1a85abd61e3d31db179801a27f56e12")  // v1.19-801.0 (unknown platform)
    {
        prehook_inject_addr = NULL;
        prehook_ret = NULL;
        get_mem_mgr_func = (void*)(dxmd_base + 0x36A10);

        textlist_installer_func = (void*)(dxmd_base + 0x3656E0);
        textlist_str_alloc_call_instruction = (void*)(dxmd_base + 0x362F9B);
        textlist_str_alloc_func = (void*)(dxmd_base + 0x2A740);

        loadingscreen_startsubs_func = (void*)(dxmd_base + 0xED7080);
        loadingscreen_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0xED713D);
        str_eq_operator_func = (void*)(dxmd_base + 0x26D60);
        uielement_playvid_func = (void*)(dxmd_base + 0x141B1E0);
        submgr_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0x3676E7);
        vidscreen_init_func = (void*)(dxmd_base + 0x1324850);
        menuscreen_init_func = (void*)(dxmd_base + 0x7AD3A0);
    }
    else if (exe_md5 == "a47cbe45e694dd57ec0d141fd7854589")  // Breach v1.15-758.0 Steam
    {
        prehook_inject_addr = NULL;
        prehook_ret = NULL;
        get_mem_mgr_func = (void*)(dxmd_base + 0x36890);

        textlist_installer_func = (void*)(dxmd_base + 0x365840);
        textlist_str_alloc_call_instruction = (void*)(dxmd_base + 0x36316B);
        textlist_str_alloc_func = (void*)(dxmd_base + 0x2A650);

        loadingscreen_startsubs_func = (void*)(dxmd_base + 0xED7770);
        loadingscreen_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0xED782D);
        str_eq_operator_func = (void*)(dxmd_base + 0x26C60);
        uielement_playvid_func = (void*)(dxmd_base + 0x1419AE0);
        submgr_startsubs_get_subs_data_instr = (void*)(dxmd_base + 0x367C07);
        vidscreen_init_func = (void*)(dxmd_base + 0x1323390);
        menuscreen_init_func = (void*)(dxmd_base + 0x7ACEF0);  // Called a few instructions after vidscreen_init_func
    }
    else
    {
        debug.print("[ERROR] Unrecognized game version. Exiting...\n");
        MessageBox(NULL, ("Unrecognized game version:\n"+ exe_md5).c_str(), "ERROR", MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_APPLMODAL);
        ExitProcess(SP_ERR_BAD_FILE_TYPE);
    }

    install_pre_hook();

    translations_enabled = GetPrivateProfileInt("Language", "EnableTranslation", 1, cfg_file);
    debug.print("Language translation is ");
    if (translations_enabled)
    {
        debug.print("enabled.\n");
    }
    else
    {
        debug.print("disabled.\n");
    }

    char cfg_str[MAX_PATH];
    GetPrivateProfileString("Language", "StringsJSON", "", cfg_str, MAX_PATH, cfg_file);
    load_translation_json(cfg_str);

    // @TODO: Delete this when the first-play cutscene translation bug is fixed
    if (GetPrivateProfileInt("Language", "CutsceneBugFixWarning", 1, cfg_file))
    {
        std::string warning = json["dev_messages"]["cutscene_first_play_warning"].get<std::string>();
        WCHAR wide_char_buf[512];
        MultiByteToWideChar(CP_UTF8, 0, warning.c_str(), (int)warning.size(), wide_char_buf, 512);
        MessageBoxW(NULL, wide_char_buf, L"WARNING", MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_APPLMODAL);
    }
}


extern "C" uint64_t textlist_res_id;
extern "C" uint32_t textlist_str_id;
extern "C" uint64_t video_res_id;


DWORD WINAPI async_thread(LPVOID param)
{
    bool running = true;

    debug.print("Starting asynchronous loop...\n");
    while (running)
    {
        static uint64_t last_res_id = -1;
        static uint32_t last_str_id = -1;
        static uint64_t last_video_id = -1;
        if (textlist_res_id != last_res_id)
        {
            //debug.print(sp::str::format("Loaded resource with ID: %" PRIx64 "\n", textlist_res_id));
        }
        if (textlist_str_id != last_str_id)
        {
            //debug.print(sp::str::format("Loaded string with ID: %" PRIx64 "\n", textlist_str_id));
        }
        if (video_res_id != last_video_id && video_res_id  != 0xffffffffffffffff  && video_res_id != 0)
        {
            debug.print(sp::str::format("Loaded video with ID: %" PRIx64 "\n", video_res_id));
        }
        last_res_id = textlist_res_id;
        last_str_id = textlist_str_id;
        last_video_id = video_res_id;
        
        Sleep(100);
    }
    return 0;
}
