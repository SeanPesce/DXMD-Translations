; Author: Sean Pesce

.code

extern textlist_installer_func:QWORD
extern install_translation_hooks_ptr:QWORD
extern prehook_inject_addr:QWORD
extern prehook_ret:QWORD

hook_installer proc
    
    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp

    ; Check whether the TextList installer has been deobfuscated
    mov rax, 190  ; 0xBE
    mov rbx, QWORD PTR [textlist_installer_func]
    movzx rbx, BYTE PTR [rbx]
    cmp rax, rbx
    je hook_installer_lbl_restore_registers

    call install_translation_hooks_ptr

    ; Remove the jump to this function now that the hooks have been installed
    ; Original bytes at 0x14805e513: 40 8A F3 66 FF CE 50 0F  B7 C4 40 FE C6 41 57 41
    mov rax, QWORD PTR [prehook_inject_addr]  ; 0x14805e513
    mov rbx, 1103609505044990528  ; 0x0f50ceff66f38a40
    mov QWORD PTR [rax], rbx
    add rax, 8
    mov rbx, 4708304258364130487  ; 0x415741c6fe40c4b7
    mov QWORD PTR [rax], rbx

    hook_installer_lbl_restore_registers:
    ; Restore registers
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Original code
    mov sil, bl
    dec si
    push rax
    movzx eax, sp
    inc sil
    push r15

    ; Return to original execution point
    jmp prehook_ret ; 0x14805E522
hook_installer endp


extern textlist_res_id:QWORD
extern get_mem_mgr_func:QWORD
extern textlist_installer_hook_ret:QWORD

textlist_installer_hook proc
    ; Store current resource ID
    ; pending_res->m_pResource->m_pResourceStub->m_ridResource
    push rax
    mov rax, rdx
    mov rax, QWORD PTR [rax]
    add rax, 8
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [textlist_res_id], rax
    pop rax

    ; Original function start
    mov QWORD PTR [rsp+8], rbx
    push rdi
    sub rsp, 32 ; 0x20
    mov rdi, rdx
    call get_mem_mgr_func

    ; Jump back to original function
    jmp textlist_installer_hook_ret
textlist_installer_hook endp


extern translations_enabled:BYTE
extern textlist_str_id:DWORD
extern textlist_str_len:QWORD
extern textlist_str_buf_ptr:QWORD
extern textlist_res_reader:QWORD
extern textlist_str_alloc_func:QWORD
extern textlist_str_alloc_hook_ret:QWORD
extern store_textlist_str_info_ptr:QWORD

textlist_str_alloc_hook proc
    ; Store pointer to resource reader
    mov QWORD PTR [textlist_res_reader], r15
    
    ; Restore pointer to resource reader
    mov r15, QWORD PTR [textlist_res_reader]

    ; Store current string ID, length, and pointer
    push rax
    ; uint32_t rdi == Original string ID pointer
    mov eax, DWORD PTR [rdi]
    mov DWORD PTR [textlist_str_id], eax
    ; uint64_t r8 == Original string length
    mov QWORD PTR [textlist_str_len], r8
    ; uint64_t rdx == Original string pointer
    mov QWORD PTR [textlist_str_buf_ptr], rdx
    pop rax

    ; Check if translations are disabled
    push rax
    mov al, BYTE PTR [translations_enabled]
    cmp al, 0
    pop rax
    je lbl_overwrite_string_data

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp

    ; Load translated string
    call store_textlist_str_info_ptr

    ; Restore registers
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    lbl_overwrite_string_data:
    ; Overwrite pointer and length with translated string/length
    ; uint64_t r8 == Original string length
    mov r8, QWORD PTR [textlist_str_len]
    ; uint64_t rdx == Original string pointer
    mov rdx, QWORD PTR [textlist_str_buf_ptr]

    ; Original instructions
    call textlist_str_alloc_func
    lea r8, [rsp+48]  ; 0x30
    lea rdx, [rsp+32]  ; 0x20
    
    ; Restore pointer to resource reader
    mov r15, QWORD PTR [textlist_res_reader]

    ; Jump back to original function
    jmp textlist_str_alloc_hook_ret
textlist_str_alloc_hook endp


extern video_res_id:QWORD
extern loadingscreen_video_id_hook_ret:QWORD

loadingscreen_video_id_hook proc
    ; Store video ID
    push rax
    mov rax, rcx
    add rax, 152  ; 0x98
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [video_res_id], rax
    pop rax

    ; Update flag that indicates whether a non-loading-screen video is playing
    mov BYTE PTR [playing_video_no_load_screen], 0

    ; Original function start
    mov QWORD PTR [rsp+8], rbx
    mov QWORD PTR [rsp+16], rsi
    push rdi
    sub rsp, 96  ; 0x60

    ; Jump back to original function
    jmp loadingscreen_video_id_hook_ret
loadingscreen_video_id_hook endp


extern loadingscreen_subs_data_hook_ret:QWORD
extern submgr_startsubs_data_hook_ret:QWORD
extern subtitles_zstr_buf_ptr:QWORD
extern str_eq_operator_func:QWORD
extern translate_next_subtitle:BYTE
extern store_subtitle_str_info_ptr:QWORD

loadingscreen_subs_data_hook proc
    ; Original instructions
    lea rdx, [rdi+8]
    lea rcx, [rsp+32]

    ; Check if translations are disabled
    push rax
    mov al, BYTE PTR [translations_enabled]
    cmp al, 0
    pop rax
    je lbl_call_str_eq_operator

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp

    ; Load translated string
    call store_subtitle_str_info_ptr

    ; Restore registers
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Check if next subtitle should be translated
    push rax
    mov al, BYTE PTR [translate_next_subtitle]
    cmp al, 0
    pop rax
    je lbl_call_str_eq_operator

    ; Overwrite pointer to subtitles data
    ;; rdx holds address of ZString
    mov rdx, QWORD PTR [subtitles_zstr_buf_ptr]

    lbl_call_str_eq_operator:
    ; Original instruction
    call str_eq_operator_func

    ; Jump back to original function
    jmp loadingscreen_subs_data_hook_ret
loadingscreen_subs_data_hook endp


extern playing_video_no_load_screen:BYTE

submgr_data_hook proc
    ; Original instructions
    mov rax, QWORD PTR [rbp-105]  ; 0x69
    lea rcx, [rsi+16]
    mov QWORD PTR [rsi+8], rax

    ; Check if translations are disabled
    push rax
    mov al, BYTE PTR [translations_enabled]
    cmp al, 0
    pop rax
    je lbl_call_str_eq_operator

    ; Check if video is playing
    push rax
    mov al, BYTE PTR [playing_video_no_load_screen]
    cmp al, 0
    pop rax
    mov BYTE PTR [playing_video_no_load_screen], 0
    je lbl_call_str_eq_operator

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp

    ; Load translated string
    call store_subtitle_str_info_ptr

    ; Restore registers
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Check if next subtitle should be translated
    push rax
    mov al, BYTE PTR [translate_next_subtitle]
    cmp al, 0
    pop rax
    je lbl_call_str_eq_operator

    ; Overwrite pointer to subtitles data
    ;; rdx holds address of ZString
    mov rdx, QWORD PTR [subtitles_zstr_buf_ptr]

    lbl_call_str_eq_operator:
    ; Original instruction
    call str_eq_operator_func

    ; Jump back to original function
    jmp submgr_startsubs_data_hook_ret
submgr_data_hook endp


extern uielement_video_id_hook_ret:QWORD

uielement_video_id_hook proc
    ; Store video ID
    push rax
    mov rax, rcx
    add rax, 224  ; 0xe0
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [video_res_id], rax
    pop rax

    ; Update flag that indicates whether a non-loading-screen video is playing
    mov BYTE PTR [playing_video_no_load_screen], 1

    ; Original function start
    mov QWORD PTR [rsp+16], rbx
    mov QWORD PTR [rsp+24], rsi
    push rdi
    sub rsp, 112  ; 0x70

    ; Jump back to original function
    jmp uielement_video_id_hook_ret
uielement_video_id_hook endp


extern vidscreen_init_hook_ret:QWORD
extern menuscreen_init_func:QWORD

vidscreen_init_hook proc
    ; Store video ID
    push rax
    mov rax, rcx
    add rax, 328  ; 0x148
    mov rax, QWORD PTR [rax]
    add rax, 296  ; 0x128
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [video_res_id], rax
    pop rax

    ; Original function start
    mov QWORD PTR [rsp+8], rbx
    push rdi
    sub rsp, 80  ; 0x50
    mov rdi, rcx
    call menuscreen_init_func

    ; Update flag that indicates whether a non-loading-screen video is playing
    mov BYTE PTR [playing_video_no_load_screen], 1

    ; Jump back to original function
    jmp vidscreen_init_hook_ret
vidscreen_init_hook endp


extern uicredits_video_id_hook_ret:QWORD

uicredits_video_id_hook proc
    ; Store video ID
    push rax
    mov rax, rcx
    add rax, 104  ; 0x68
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [video_res_id], rax
    pop rax

    ; Update flag that indicates whether a non-loading-screen video is playing
    mov BYTE PTR [playing_video_no_load_screen], 1

    ; Original function start
    mov QWORD PTR [rsp+8], rbx
    push rdi
    sub rsp, 32  ; 0x20
    mov rdi, rcx
    mov rcx, QWORD PTR [rcx+40]  ; 0x28

    ; Jump back to original function
    jmp uicredits_video_id_hook_ret
uicredits_video_id_hook endp


extern renderplayer_video_id_hook_ret:QWORD

renderplayer_video_id_hook proc
    ; If already playing a video, do nothing (otherwise menu cutscene ID will be overwritten)
    push rax
    mov al, BYTE PTR [playing_video_no_load_screen]
    cmp al, 1
    pop rax
    je lbl_renderplayer_video_id_hook_orig_instrs
    
    ; Store video ID
    push rax
    mov rax, rdx
    add rax, 144  ; 0x90
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [video_res_id], rax
    pop rax

    ; Update flag that indicates whether a non-loading-screen video is playing
    mov BYTE PTR [playing_video_no_load_screen], 1

    ; Original function start
    lbl_renderplayer_video_id_hook_orig_instrs:
    mov rcx, rdi
    movaps XMMWORD PTR [rbp-32], xmm0
    call QWORD PTR [rax+40]  ; 0x28
    mov QWORD PTR [rax-48], rsi  ; 0x30

    ; Jump back to original function
    jmp renderplayer_video_id_hook_ret
renderplayer_video_id_hook endp


extern resid_record_mapping_hook_ret:QWORD
extern cur_mapping_runtime_id:QWORD
extern cur_mapping_res_id:QWORD
extern print_res_mapping_info_ptr:QWORD

resid_record_mapping_hook proc
    ; Store runtime ID
    push rax
    mov rax, QWORD PTR [rdx]
    mov QWORD PTR [cur_mapping_runtime_id], rax

    ; Store res ID
    mov rax, QWORD PTR [r8]
    mov QWORD PTR [cur_mapping_res_id], rax
    pop rax

    ; Save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push rbp

    ; Print mapping
    call print_res_mapping_info_ptr

    ; Restore registers
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Original function start
    mov QWORD PTR [rsp+16], rbx
    mov QWORD PTR [rsp+24], rbp  ; 0x18
    push rsi
    push rdi
    push r14

    ; Jump back to original function
    jmp resid_record_mapping_hook_ret
resid_record_mapping_hook endp

end
