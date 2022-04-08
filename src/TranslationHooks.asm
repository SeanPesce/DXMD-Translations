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

end
