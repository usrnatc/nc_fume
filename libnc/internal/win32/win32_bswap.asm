;; win32_bswap.asm
;;
;; This is intended as a replacement to the MS _byteswap_xxxx functions
;; This is unneeded if not build with /NODEFAULTLIB
;;

.code

_byteswap_ulong proc
    mov     eax, ecx
    bswap   eax
    bnd ret
_byteswap_ulong endp

_byteswap_uint64 proc
    mov     rax, rcx
    bswap   rax
    bnd ret
_byteswap_uint64 endp

_byteswap_ushort proc
    movzx   eax, cx
    rol     ax,  8
    bnd ret
_byteswap_ushort endp

end
