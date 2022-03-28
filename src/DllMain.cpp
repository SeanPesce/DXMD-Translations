// Author: Sean Pesce

#include <Windows.h>

#include <inttypes.h>

#include "sp/environment.h"
#include "sp/file.h"
#include "sp/io/powershell_ostream.h"

const char *cfg_file = ".\\retail\\DXMD_Mod.ini";
sp::io::ps_ostream debug;

// Configurable settings
std::string log_file = ".\\retail\\DXMD_Mod.log";
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
        get_module_size(GetModuleHandle(NULL), (LPVOID*)&dxmd_base, &dxmd_size); // Obtain DXMD base address & size
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
        }

        debug.print("Initializing keybinds & additional settings...\n");
        init_settings();
        debug.print("Finished reading keybinds & settings.\n");

        // Initialize thread(s)
        debug.print("Initializing FoV changer thread...\n");
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


// Parses DXMD_FOV.ini for intialization settings
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
    return 0; // Return 0 on success
}


// Version-specific memory addresses
extern "C" void* prehook_inject_addr;
extern "C" void* prehook_ret;
extern "C" void* textlist_installer_func;
extern "C" void* get_mem_mgr_func;
extern void* str_alloc_call_instruction;
extern "C" void* str_alloc_func;

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
        prehook_inject_addr = (void*)(dxmd_base + 0x805e513);
        prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);  // DXMD.exe+0x805E522
        textlist_installer_func = (void*)(dxmd_base + 0x36A39C0);
        get_mem_mgr_func = (void*)(dxmd_base + 0x3159300);
        str_alloc_call_instruction = (void*)(dxmd_base + 0x36A101A);
        str_alloc_func = (void*)(dxmd_base + 0x314C0C0);
    }
    else if (exe_md5 == "3745fa30bf3f607a58775f818c5e0ac0")  // v1.19-801.0 GoG
    {
        prehook_inject_addr = NULL;
        prehook_ret = NULL;
        textlist_installer_func = (void*)(dxmd_base + 0x3656E0);
        get_mem_mgr_func = (void*)(dxmd_base + 0x36A10);
        str_alloc_call_instruction = (void*)(dxmd_base + 0x362F9B);
        str_alloc_func = (void*)(dxmd_base + 0x2A740);
    }
    /*else if (exe_md5 == "c1a85abd61e3d31db179801a27f56e12")  // v1.19-801.0 (unknown platform)
    {
        // @TODO: VirtualProtect fails on this version of the executable. Why?
        prehook_inject_addr = (void*)(0x14805e513);
        prehook_ret = (void*)((uint64_t)prehook_inject_addr + 15);  // DXMD.exe+0x805E522
        LPVOID temp = prehook_inject_addr;
        ULONG sz = 16;
        ULONG old_protect = 0;
        PULONG old_protect_ptr = &old_protect;
        typedef NTSTATUS(*NtProtectVirtualMemory_t)(HANDLE, PVOID, PULONG, ULONG, PULONG);
        NtProtectVirtualMemory_t NtProtectVirtualMemory  = (NtProtectVirtualMemory_t)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtProtectVirtualMemory");
        NTSTATUS status = NtProtectVirtualMemory(GetCurrentProcess(), &temp, &sz, PAGE_EXECUTE_READWRITE, old_protect_ptr);
        textlist_installer_func = (void*)(0x1436A39C0);
        get_mem_mgr_func = (void*)(0x143159300);
        str_alloc_call_instruction = (void*)(0x1436A101A);
        str_alloc_func = (void*)(0x14314C0C0);
    }*/
    else
    {
        debug.print("[ERROR] Unrecognized game version. Exiting...\n");
        MessageBox(NULL, ("Unrecognized game version:\n"+ exe_md5).c_str(), "ERROR", MB_OK);
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
}


extern "C" uint64_t resource_id;
extern "C" uint32_t string_id;


DWORD WINAPI async_thread(LPVOID param)
{
    bool running = true;

    debug.print("Starting FoV modifier loop...\n");
    while (running)
    {
        static uint64_t last_res_id = -1;
        static uint32_t last_str_id = -1;
        if (resource_id != last_res_id) {
            debug.print(sp::str::format("Loaded resource with ID: %" PRIx64 "\n", resource_id));
        }
        if (string_id != last_str_id) {
            debug.print(sp::str::format("Loaded string with ID: %" PRIx64 "\n", string_id));
        }
        last_res_id = resource_id;
        last_str_id = string_id;
        
        Sleep(100);
    }
    return 0;
}
