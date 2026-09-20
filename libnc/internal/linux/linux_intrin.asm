;; linux_intrin.asm
;;
;; These are intended as minimal implementations of miscellaneous intrinsic
;; functions for Linux (System V ABI)
;;

default rel
section .text
bits 64

global BitScanForward64
align 16
BitScanForward64:
    bsf     rax, rdi
    ret

global BitScanReverse64
align 16
BitScanReverse64:
    bsr     rax, rdi
    ret

global PopCount64
align 16
PopCount64:
    popcnt  rax, rdi
    ret

global MemZero
align 16
MemZero:
    xor     eax, eax
    mov     rcx, rsi
    rep stosb
    ret
