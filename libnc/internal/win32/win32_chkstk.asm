;; win32_chkstk.asm
;;
;; This is intended as a replacement to the MS C Runtime __chkstk method
;; This is unneeded if not build with /NODEFAULTLIB
;;

.code

__chkstk proc
    sub         rsp, 16
    mov         qword ptr [rsp], r10
    mov         qword ptr [rsp + 8], r11
    xor         r11, r11
    lea         r10, [rsp + 24]
    sub         r10, rax
    cmovb       r10, r11
    mov         r11, qword ptr gs:[16]
    cmp         r10, r11
    bnd jae     @done
    and         r10w, 0F000h

@force_commit_page:
    lea         r11, [r11 - 4096]
    mov         byte ptr [r11], 0
    cmp         r10, r11
    bnd jne     @force_commit_page

@done:
    mov         r10, qword ptr [rsp]
    mov         r11, qword ptr [rsp + 8]
    add         rsp, 16
    bnd ret
__chkstk endp

end
