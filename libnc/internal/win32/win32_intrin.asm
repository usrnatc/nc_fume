;; win32_intrin.asm
;;
;; These are intended as minimal implementations of miscellaneous intrinsic
;; functions
;;

.code

align(16)
BitScanForward64 proc
    bsf         rax, rcx
    ret
BitScanForward64 endp

align(16)
BitScanReverse64 proc
    bsr         rax, rcx
    ret
BitScanReverse64 endp

align(16)
PopCount64 proc
    popcnt      rax, rcx
    ret
PopCount64 endp

align(16)
MemZero proc
    push        rdi
    mov         rdi, rcx
    xor         eax, eax
    mov         rcx, rdx
    rep stosb
    pop         rdi
    bnd ret
MemZero endp

end
