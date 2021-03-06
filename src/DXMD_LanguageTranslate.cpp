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


extern const char* cfg_file;
extern sp::io::ps_ostream debug;

extern "C" uint64_t textlist_res_id;
extern "C" uint32_t textlist_str_id;
extern uint8_t textlist_str_buf[_TEXTLIST_STR_BUF_LEN];
extern "C" void* textlist_str_buf_ptr;
extern "C" uint64_t textlist_str_len;


std::map<uint64_t, std::map<uint32_t, std::string>> textlist_translation_data;
// Map video/cinematic IDs to timed subtitle data
std::map<uint64_t, std::string> subtitle_translation_data;


extern "C" void hook_installer();
extern "C" void textlist_installer_hook();
extern "C" void textlist_str_alloc_hook();
extern "C" void loadingscreen_video_id_hook();
extern "C" void loadingscreen_subs_data_hook();
extern "C" void uielement_video_id_hook();
extern "C" void uicredits_video_id_hook();
extern "C" void submgr_data_hook();
extern "C" void vidscreen_init_hook();
extern "C" void renderplayer_video_id_hook();
extern "C" void ui_font_addr_hook();
extern "C" void ui_font_replace_hook();

extern "C" void resid_record_mapping_hook();


// This function was developed based on code from Franc[e]sco (from ccplz.net)
BOOL get_module_size(HMODULE hmodule, LPVOID* lplp_base, PDWORD64 lpdw_size)
{
    if (hmodule == GetModuleHandle(NULL))
    {
        PIMAGE_NT_HEADERS img_nt_hdrs = ImageNtHeader((PVOID)hmodule);

        if (img_nt_hdrs == NULL)
        {
            return FALSE;
        }

        *lplp_base = (LPVOID)hmodule;
        *lpdw_size = img_nt_hdrs->OptionalHeader.SizeOfImage;
    }
    else
    {
        MODULEINFO  module_info;

        if (!GetModuleInformation(GetCurrentProcess(), hmodule, &module_info, sizeof(MODULEINFO)))
        {
            return FALSE;
        }

        *lplp_base = module_info.lpBaseOfDll;
        *lpdw_size = module_info.SizeOfImage;
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

    // Erase any existing translation data
    textlist_translation_data = std::map<uint64_t, std::map<uint32_t, std::string>>();
    subtitle_translation_data = std::map<uint64_t, std::string>();

    size_t file_count = 0;

    // Parse textlist data
    if (json.contains("textlists"))
    {
        size_t file_count = json["textlists"].size();
        for (size_t i = 0; i < file_count; i++)
        {
            auto f = json["textlists"][i];
            if (f.contains("id") && f.contains("content"))
            {
                size_t str_count = f["content"].size();
                std::map<uint32_t, std::string> textlist_data;

                for (size_t j = 0; j < str_count; j++)
                {
                    auto s = f["content"][j];
                    if (s.contains("id") && s.contains("string"))
                    {
                        textlist_data.insert({ s["id"].get<uint32_t>(), s["string"].get<std::string>() });
                    }
                    else
                    {
                        debug.print("[WARNING] Missing textlists[" + std::to_string(i) + "].content[" + std::to_string(j) + "] ID or string data\n");
                    }
                }

                textlist_translation_data.insert({ f["id"].get<uint64_t>(), textlist_data });
            }
            else
            {
                debug.print("[WARNING] Missing textlists[" + std::to_string(i) + "] content or ID\n");
            }
        }
        debug.print("Finished loading translation data.\n");
    }
    else
    {
        debug.print("[WARNING] Failed to find TextList JSON data. Most data will not be translated.\n");
    }

    // Parse video subtitle data
    size_t missing_vid_ids = 0;
    if (json.contains("subtitles"))
    {
        file_count = json["subtitles"].size();
        for (size_t i = 0; i < file_count; i++)
        {
            auto reslib = json["subtitles"][i];
            if (reslib.contains("content"))
            {
                size_t vid_count = reslib["content"].size();
                for (size_t j = 0; j < vid_count; j++)
                {
                    auto vid = reslib["content"][j];
                    uint64_t id = 0;
                    if (vid.contains("video_id"))
                    {
                        id = vid["video_id"].get<uint64_t>();
                        if (id == 0 || id == 0xffffffffffffffff)
                        {
                            missing_vid_ids++;
                            debug.print("No ID for video " + reslib["id"].get<std::string>() + "[" + std::to_string(j) + "]\n");
                            continue;
                        }

                        if (vid.contains("subs"))
                        {
                            size_t line_count = vid["subs"].size();
                            std::string subs_data;
                            for (size_t k = 0; k < line_count; k++)
                            {
                                auto sub = vid["subs"][k];
                                if (sub.contains("start") && sub.contains("end") && sub.contains("string"))
                                {
                                    // Build timestamp header
                                    std::string start = sub["start"].get<std::string>();
                                    std::string stop = sub["end"].get<std::string>();
                                    subs_data += "//(" + start;
                                    if (!stop.empty())
                                    {
                                        subs_data += ",";
                                    }
                                    subs_data += stop + ")\\\\";
                                    // Add subtitle
                                    subs_data += sub["string"].get<std::string>();
                                }
                                else
                                {
                                    debug.print("[WARNING] Missing subtitles[" + std::to_string(i) + "].content[" + std::to_string(j) + "].subs start, end, or string data\n");
                                }
                            }
                            subtitle_translation_data.insert({ id, subs_data });
                            //debug.print("\n\n"+subs_data + "\n\n");
                        }
                        else
                        {
                            debug.print("[WARNING] Missing subtitles[" + std::to_string(i) + "].content[" + std::to_string(j) + "].subs\n");
                        }
                    }
                    else
                    {
                        debug.print("[WARNING] Missing subtitles[" + std::to_string(i) + "].content[" + std::to_string(j) + "].video_id\n");
                        continue;
                    }
                }
            }
            else
            {
                debug.print("[WARNING] Missing subtitles[" + std::to_string(i) + "].content\n");
            }
        }
    }
    else
    {
        debug.print("[WARNING] Failed to find subtitles JSON data. Cutscenes not be translated.\n");
    }
    if (missing_vid_ids > 0)
    {
        debug.print("[WARNING] Missing IDs for " + std::to_string(missing_vid_ids) + " videos. Subtitles for these cinematics will not be translated.\n");
    }

    // @TODO: Delete this if/when the first-play cutscene translation bug is fixed
    if (GetPrivateProfileInt("Language", "CutsceneBugFixWarning", 1, cfg_file))
    {
        debug.print("CutsceneBugFixWarning=1\n");

        if (json.contains("dev_messages") && json["dev_messages"].contains("cutscene_first_play_warning"))
        {
            std::string warning = json["dev_messages"]["cutscene_first_play_warning"].get<std::string>();
            WCHAR wide_char_buf[2048];
            MultiByteToWideChar(CP_UTF8, 0, warning.c_str(), (int)warning.size(), wide_char_buf, 2048);
            MessageBoxW(NULL, wide_char_buf, L"WARNING", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL);
        }
        else
        {
            debug.print("[WARNING] Failed to find CutsceneBugFixWarning message JSON field.\n");
        }
    }
    else
    {
        debug.print("CutsceneBugFixWarning=0\n");
    }
}



void load_ui_font(const char* fpath)
{
    if (!fpath || !strnlen_s(fpath, MAX_PATH))
    {
        ui_font_data_size = 0;
        debug.print("No UI font file specified.\n");
        return;
    }

    debug.print("Loading UI font file: " + std::string(fpath) + "\n");

    // Check if file exists
    std::ifstream in_file(fpath, std::ios::in|std::ios::binary);
    if (!in_file.good())
    {
        ui_font_data_size = 0;
        std::string err_msg = "Failed to find UI font file:\n  " + std::string(fpath) + "\n";
        debug.print(err_msg);
        MessageBox(NULL, err_msg.c_str(), "WARNING", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL);
        return;
    }

    // Get file size
    in_file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize file_sz = in_file.gcount();
    in_file.clear();  // Reset EOF flag
    in_file.seekg(0, std::ios_base::beg);

    if (file_sz > sizeof(ui_font_buf))
    {
        ui_font_data_size = 0;
        std::string err_msg = "UI font file exceeds buffer size of " + std::to_string(sizeof(ui_font_buf)) + " bytes. Contact the developer to increase the buffer size.";
        debug.print(err_msg);
        MessageBox(NULL, err_msg.c_str(), "WARNING", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL);
        in_file.close();
        return;
    }

    in_file.read((char*)ui_font_buf, file_sz);
    if (in_file.fail())
    {
        ui_font_data_size = 0;
        std::string err_msg = "Error when reading data from UI font file:\n  " + std::string(fpath) + "\n";
        debug.print(err_msg);
        MessageBox(NULL, err_msg.c_str(), "WARNING", MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL);
        return;
    }

    ui_font_data_size = file_sz;
}



// Returns 0 if the string was found; returns 1 otherwise
extern "C" int get_textlist_translation_string(uint64_t res_id, uint32_t str_id, std::string& out)
{
    //debug.print("Loading string: " + std::to_string(res_id) + "->" + std::to_string(str_id) + "\n");
    if (textlist_translation_data.find(res_id) != textlist_translation_data.end())
    {
        //debug.print("Found resource ID.\n");
        auto textlist_data = textlist_translation_data[res_id];
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


// Returns 0 if the string was found; returns 1 otherwise
extern "C" int write_subtitle_translation_string(uint64_t vid_id)
{
    if (subtitle_translation_data.find(vid_id) != subtitle_translation_data.end())
    {
        std::string subtitle_data = subtitle_translation_data[vid_id];

        // Write global subtitles replacement buffer:

        //   Write pointer to string buffer
        char** subtitles_str_ptr = (char**)subtitles_str_buf;
        *subtitles_str_ptr = subtitles_str_buf_ptr;

        //   Write string length
        uint32_t* subtitles_sz_ptr = (uint32_t*)(subtitles_str_buf + 8);
        *subtitles_sz_ptr = (uint32_t)subtitle_data.size();

        //   Write buffer capacity
        uint32_t* subtitles_capacity_ptr = (uint32_t*)(subtitles_str_buf + 12);
        *subtitles_capacity_ptr = subtitles_capacity;

        //   Write subtitles string data
        memcpy_s(subtitles_str_buf_ptr, subtitles_capacity, subtitle_data.c_str(), subtitle_data.size());

        //   Write null terminator for string
        uint8_t* str_end = (uint8_t*)subtitles_str_buf_ptr + subtitle_data.size();
        *str_end = 0;

        return 0;
    }
    // Failed to find the string
    //debug.print("String not found.\n");
    return 1;
}


// Install hooks in TextList loader and language string allocator
extern "C" void install_translation_hooks()
{
    // TextList hooks
    sp::mem::code::x64::inject_jmp_14b(textlist_installer_func, &textlist_installer_hook_ret, 4, textlist_installer_hook);
    sp::mem::code::x64::inject_jmp_14b(textlist_str_alloc_call_instruction, &textlist_str_alloc_hook_ret, 1, textlist_str_alloc_hook);

    // Video subtitle hooks
    sp::mem::code::x64::inject_jmp_14b(loadingscreen_startsubs_func, &loadingscreen_video_id_hook_ret, 1, loadingscreen_video_id_hook);
    sp::mem::code::x64::inject_jmp_14b(uielement_playvid_func, &uielement_video_id_hook_ret, 1, uielement_video_id_hook);
    sp::mem::code::x64::inject_jmp_14b(uicredits_playvid_func, &uicredits_video_id_hook_ret, 3, uicredits_video_id_hook);
    sp::mem::code::x64::inject_jmp_14b(loadingscreen_startsubs_get_subs_data_instr, &loadingscreen_subs_data_hook_ret, 0, loadingscreen_subs_data_hook);
    sp::mem::code::x64::inject_jmp_14b(submgr_startsubs_get_subs_data_instr, &submgr_startsubs_data_hook_ret, 3, submgr_data_hook);
    sp::mem::code::x64::inject_jmp_14b(vidscreen_init_func, &vidscreen_init_hook_ret, 4, vidscreen_init_hook);
    sp::mem::code::x64::inject_jmp_14b(renderplayer_start_hook_addr, &renderplayer_video_id_hook_ret, 0, renderplayer_video_id_hook);

    // UI font hooks
    sp::mem::code::x64::inject_jmp_14b(ui_font_addr_hook_addr, &ui_font_addr_hook_ret, 1, ui_font_addr_hook);
    sp::mem::code::x64::inject_jmp_14b(ui_font_replace_hook_addr, &ui_font_replace_hook_ret, 1, ui_font_replace_hook);

    if (resid_record_mapping_func && debug_resid_map)
    {
        sp::mem::code::x64::inject_jmp_14b(resid_record_mapping_func, &resid_record_mapping_hook_ret, 0, resid_record_mapping_hook);
    }
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
    // sp::mem::code::x64::inject_jmp_14b(prehook_inject_addr, &prehook_ret, 1, hook_installer);
    // Original bytes at 0x14805e513: 40 8A F3 66 FF CE 50 0F B7 C4 40 FE C6 41 57 41
    sp::mem::set_protection((void*)prehook_inject_addr, 15 + 1, MEM_PROTECT_RWX);
    uint8_t prehook_jmp[15] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x90 };  // jmp hook_installer
    // Insert function address as operand in pre-hook jmp instruction
    *(uint64_t*)(&prehook_jmp[6]) = (uint64_t)(&hook_installer);
    // Write the jmp instruction
    memcpy((void*)prehook_inject_addr, prehook_jmp, 15);
}


// Called from TextList loader hook; stores translated string information for later replacement
extern "C" void store_textlist_str_info()
{
    std::string tmp_str;
    int retval = get_textlist_translation_string(textlist_res_id, textlist_str_id, tmp_str);
    if (retval)
    {
        // String was not found; don't change original string information
        textlist_str_buf[0] = 0;
        return;
    }

    // String was found
    textlist_str_buf_ptr = &textlist_str_buf;
    textlist_str_len = tmp_str.size();
    memcpy_s(textlist_str_buf, sizeof(textlist_str_buf), tmp_str.c_str(), tmp_str.size());

    // Probably not necessary, but null-terminate the string just in case
    if (tmp_str.size() < sizeof(textlist_str_buf))
    {
        textlist_str_buf[tmp_str.size()] = 0;
    }
    else
    {
        textlist_str_buf[sizeof(textlist_str_buf) - 1] = 0;
    }
}
extern "C" void* store_textlist_str_info_ptr = &store_textlist_str_info;


// Called from video subtitle hook; stores translated string information for later replacement
extern "C" void store_subtitle_str_info()
{
    translate_next_subtitle = !write_subtitle_translation_string(video_res_id);
}
extern "C" void* store_subtitle_str_info_ptr = &store_subtitle_str_info;


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


extern "C" void print_res_mapping_info()
{
    debug.print(std::to_string(cur_mapping_runtime_id) + "->" + std::string(cur_mapping_res_id) + "\n");
}
extern "C" void* print_res_mapping_info_ptr = &print_res_mapping_info;
