;; linux_math.asm
;;
;; These are intended as minimal implementations of some maths functions
;;

default rel
section .text
bits 64

;; 16-bit______________________________________________________________________
global Abs16
align 16
Abs16:
    mov     ax, di
    cwd
    xor     ax, dx
    sub     ax, dx
    ret

;; 32-bit______________________________________________________________________
global Abs32
align 16
Abs32:
    mov     eax, edi
    cdq
    xor     eax, edx
    sub     eax, edx
    ret

global Max32
align 16
Max32:
    mov     eax, edi
    cmp     eax, esi
    cmovl   eax, esi
    ret

global FMAdd32
align 16                                    ;; NOTE: we use 64 bit operations to maintain exact integer precision when adding
FMAdd32:
    cvtsi2sd    xmm0, edi
    cvtsi2sd    xmm1, esi
    cvtsi2sd    xmm2, edx
    vfmadd213sd xmm0, xmm1, xmm2
    vcvttsd2si  eax, xmm0
    ret

global FMAddF32
align 16
FMAddF32:
    vfmadd213ss xmm0, xmm1, xmm2
    ret

align 16
global PowF32
PowF32:
    movss   dword [rsp - 8], xmm0
    movss   dword [rsp - 16], xmm1
    fld     dword [rsp - 16]
    fld     dword [rsp - 8]
    fyl2x
    fld     st0
    frndint
    fsub    st1, st0
    fxch    st1
    f2xm1
    fld1
    faddp   st1, st0
    fscale
    fstp    st1
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global ExpF32
align 16
ExpF32:
    movss   dword [rsp - 8], xmm0
    fldl2e
    fld     dword [rsp - 8]
    fmulp   st1, st0
    fld     st0
    frndint
    fsub    st1, st0
    fxch    st1
    f2xm1
    fld1
    faddp   st1, st0
    fscale
    fstp    st1
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global LnF32
align 16
LnF32:
    movss   dword [rsp - 8], xmm0
    fldln2
    fld     dword [rsp - 8]
    fyl2x
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global Log10F32
align 16
Log10F32:
    movss   dword [rsp - 8], xmm0
    fldlg2
    fld     dword [rsp - 8]
    fyl2x
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global Log2F32
align 16
Log2F32:
    movss   dword [rsp - 8], xmm0
    fld1
    fld     dword [rsp - 8]
    fyl2x
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global AbsF32
align 16
AbsF32:
    pslld   xmm0, 1
    psrld   xmm0, 1
    ret

global SqrtF32
align 16
SqrtF32:
    sqrtss  xmm0, xmm0
    ret

global InvSqrtF32
align 16
InvSqrtF32:
    rsqrtss xmm0, xmm0
    ret

align 16
global LerpF32
LerpF32:
    subss       xmm2, xmm1
    vfmadd213ss xmm0, xmm2, xmm1
    ret

global RoundF32
align 16
RoundF32:
    roundss xmm0, xmm0, 0
    ret

global FloorF32
align 16
FloorF32:
    roundss xmm0, xmm0, 1
    ret

global CeilF32
align 16
CeilF32:
    roundss xmm0, xmm0, 2
    ret

global SinF32
align 16
SinF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fsin
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global CosF32
align 16
CosF32:
    movss    dword [rsp - 8], xmm0
    fld      dword [rsp - 8]
    fcos
    fstp     dword [rsp - 8]
    movss    xmm0, dword [rsp - 8]
    ret

global SinCosF32
align 16                                       ;; NOTE: void SinCosF32(f32 Val, OUT f32* SinRes, OUT f32* CosRes)
SinCosF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fsincos
    fstp    dword [rsi]
    fstp    dword [rdi]
    ret

global TanF32
align 16
TanF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fptan
    fstp    st0
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global ArcSinF32
align 16
ArcSinF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fld     st0
    fmul    st0, st0
    fld1
    fsubrp  st1, st0
    fsqrt
    fpatan
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global ArcCosF32
align 16
ArcCosF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fld     st0
    fmul    st0, st0
    fld1
    fsubrp  st1, st0
    fsqrt
    fxch    st1
    fpatan
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global ArcTanF32
align 16
ArcTanF32:
    movss   dword [rsp - 8], xmm0
    fld     dword [rsp - 8]
    fld1
    fpatan
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

global ArcTan2F32
align 16
ArcTan2F32:
    movss   dword [rsp - 8], xmm0
    movss   dword [rsp - 16], xmm1
    fld     dword [rsp - 8]
    fld     dword [rsp - 16]
    fpatan
    fstp    dword [rsp - 8]
    movss   xmm0, dword [rsp - 8]
    ret

;; 64-bit______________________________________________________________________
global Abs64
align 16
Abs64:
    mov     rax, rdi
    cqo
    xor     rax, rdx
    sub     rax, rdx
    ret


global Max64
align 16
Max64:
    mov     rax, rdi
    cmp     rax, rsi
    cmovl   rax, rsi
    ret


global FMAdd64
align 16
FMAdd64:
    cvtsi2sd    xmm0, rdi
    cvtsi2sd    xmm1, rsi
    cvtsi2sd    xmm2, rdx
    vfmadd213sd xmm0, xmm1, xmm2
    cvtsd2si    rax, xmm0
    ret


global FMAddF64
align 16
FMAddF64:
    vfmadd213sd xmm0, xmm1, xmm2
    ret


global PowF64
align 16
PowF64:
    movsd   qword [rsp - 8], xmm0
    movsd   qword [rsp - 16], xmm1
    fld     qword [rsp - 16]
    fld     qword [rsp - 8]
    fyl2x
    fld     st0
    frndint
    fsub    st1, st0
    fxch    st1
    f2xm1
    fld1
    faddp   st1, st0
    fscale
    fstp    st1
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global ExpF64
align 16
ExpF64:
    movsd   qword [rsp - 8], xmm0
    fldl2e
    fld     qword [rsp - 8]
    fmulp   st1, st0
    fld     st0
    frndint
    fsub    st1, st0
    fxch    st1
    f2xm1
    fld1
    faddp   st1, st0
    fscale
    fstp    st1
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global LnF64
align 16
LnF64:
    movsd   qword [rsp - 8], xmm0
    fldln2
    fld     qword [rsp - 8]
    fyl2x
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global Log10F64
align 16
Log10F64:
    movsd   qword [rsp - 8], xmm0
    fldlg2
    fld     qword [rsp - 8]
    fyl2x
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global Log2F64
align 16
Log2F64:
    movsd   qword [rsp - 8], xmm0
    fld1
    fld     qword [rsp - 8]
    fyl2x
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global SinF64
align 16
SinF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fsin
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global CosF64
align 16
CosF64:
    movsd    qword [rsp - 8], xmm0
    fld      qword [rsp - 8]
    fcos
    fstp     qword [rsp - 8]
    movsd    xmm0, qword [rsp - 8]
    ret

global SinCosF64
align 16                                       ;; NOTE: void SinCosF64(f64 Val, OUT f64* SinRes, OUT f64* CosRes)
SinCosF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fsincos
    fstp    qword [rsi]
    fstp    qword [rdi]
    ret

global TanF64
align 16
TanF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fptan
    fstp    st0
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global ArcSinF64
align 16
ArcSinF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fld     st0
    fmul    st0, st0
    fld1
    fsubrp  st1, st0
    fsqrt
    fpatan
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global ArcCosF64
align 16
ArcCosF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fld     st0
    fmul    st0, st0
    fld1
    fsubrp  st1, st0
    fsqrt
    fxch    st1
    fpatan
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global ArcTanF64
align 16
ArcTanF64:
    movsd   qword [rsp - 8], xmm0
    fld     qword [rsp - 8]
    fld1
    fpatan
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global ArcTan2F64
align 16
ArcTan2F64:
    movsd   qword [rsp - 8], xmm0
    movsd   qword [rsp - 16], xmm1
    fld     qword [rsp - 8]
    fld     qword [rsp - 16]
    fpatan
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global Round64NextPow2
align 16
Round64NextPow2:
    mov     rax, 64
    dec     rcx
    lzcnt   rcx, rcx
    sub     rax, rcx
    mov     rcx, rax
    mov     rax, 1
    shl     rax, cl
    ret

global Pow64
align 16
Pow64:
    mov     rax, 1                              ;; Result = 1
    test    rsi, rsi
    jz      @done                               ;; IF !Exp THEN return 1

@loop:
    test    sil, 1                               ;; is Exp odd?
    jz      @even
    imul    rax, rdi                            ;; Result *= Val

@even:
    shr     rsi, 1                              ;; Exp >>= 1
    jz      @done                               ;; IF !Exp THEN we are done
    imul    rdi, rdi                            ;; Val *= Val
    jmp     @loop

@done:
    ret

global AbsF64
align 16
AbsF64:
    psllq   xmm0, 1
    psrlq   xmm0, 1
    ret

global SqrtF64
align 16
SqrtF64:
    sqrtsd xmm0, xmm0
    ret

global InvSqrtF64
align 16
InvSqrtF64:
    movsd   qword [rsp - 8], xmm0
    fld1
    fld     qword [rsp - 8]
    fsqrt
    fdivp   st1, st0
    fstp    qword [rsp - 8]
    movsd   xmm0, qword [rsp - 8]
    ret

global RoundF64
align 16
RoundF64:
    roundsd xmm0, xmm0, 0
    ret

global FloorF64
align 16
FloorF64:
    roundsd xmm0, xmm0, 1
    ret

global CeilF64
align 16
CeilF64:
    roundsd xmm0, xmm0, 2
    ret

global LerpF64
align 16
LerpF64:                                    ;; NOTE: LerpF64(T, A, B) -> T is first so we can skip a movss before returning
    subsd       xmm2, xmm1
    vfmadd213sd xmm0, xmm2, xmm1
    ret

;; v2i64_______________________________________________________________________
global LerpV2I64
align 16
LerpV2I64:                                  ;; NOTE: void LerpV2I64(v2i64* Result, v2i64* A, v2i64* B, f32 T)
    movaps      xmm3, xmm0
    cvtss2sd    xmm3, xmm3
    shufpd      xmm3, xmm3, 0
    cvtsi2sd    xmm0, qword [rsi]
    cvtsi2sd    xmm1, qword [rsi - 8]
    unpcklpd    xmm0, xmm1
    cvtsi2sd    xmm1, qword [rdx]
    cvtsi2sd    xmm2, qword [rdx - 8]
    unpcklpd    xmm1, xmm2
    subpd       xmm1, xmm0
    vfmadd213pd xmm3, xmm1, xmm0
    cvtsd2si    rax, xmm3
    mov         qword [rdi], rax
    unpckhpd    xmm3, xmm3
    cvtsd2si    rax, xmm3
    mov         qword [rdi - 8], rax
    ret

;; v2i32_______________________________________________________________________
global LerpV2I32
align 16
LerpV2I32:                                  ;; NOTE: v2i32 LerpV2I32(v2i32 VecA, v2i32 VecB, f32 T)
    movaps      xmm2, xmm0
    movq        xmm0, rdi
    movq        xmm1, rsi
    cvtdq2ps    xmm0, xmm0
    cvtdq2ps    xmm1, xmm1
    shufps      xmm2, xmm2, 0
    subps       xmm1, xmm0
    vfmadd213ps xmm1, xmm2, xmm0
    cvtps2dq    xmm1, xmm1
    movq        rax, xmm1
    ret

;; v2i16_______________________________________________________________________
global LerpV2I16
align 16
LerpV2I16:                                  ;; NOTE: v2i16 LerpV2I16(v2i16 VecA, v2i16 VecB, f32 T)
    movaps      xmm2, xmm0
    movd        xmm0, edi
    movd        xmm1, esi
    pmovsxwd    xmm0, xmm0
    pmovsxwd    xmm1, xmm1
    cvtdq2ps    xmm0, xmm0
    cvtdq2ps    xmm1, xmm1
    shufps      xmm2, xmm2, 0
    subps       xmm1, xmm0
    vfmadd213ps xmm1, xmm2, xmm0
    cvtps2dq    xmm1, xmm1
    packssdw    xmm1, xmm1
    movd        eax, xmm1
    ret

;; v4i32_______________________________________________________________________
global LerpV4I32
align 16
LerpV4I32:                                  ;; NOTE: void LerpV4I32(v4i32* Result, v4i32* VecA, v4i32* VecB, f32 T)
    movaps      xmm3, xmm0
    movups      xmm0, [rsi]
    movups      xmm1, [rdx]
    cvtdq2ps    xmm0, xmm0
    cvtdq2ps    xmm1, xmm1
    shufps      xmm3, xmm3, 0
    subps       xmm1, xmm0
    vfmadd213ps xmm1, xmm3, xmm0
    cvtps2dq    xmm1, xmm1
    movups      [rdi], xmm1
    ret

;;
;; SSE
;; ____________________________________________________________________________
;;

global SSEPowF32
align 16
SSEPowF32:
    movd        eax, xmm0
    mov         ecx, eax
    shr         eax, 23
    sub         eax, 127
    cvtsi2ss    xmm2, eax
    and         ecx, 007FFFFFh
    or          ecx, 3F800000h
    movd        xmm0, ecx
    mov         eax, 3F800000h
    movd        xmm3, eax
    subss       xmm0, xmm3
    mov         eax, 3E93DD00h
    movd        xmm3, eax
    mulss       xmm3, xmm0
    mov         eax, 0BEB8AA3Bh
    movd        xmm4, eax
    addss       xmm3, xmm4
    mulss       xmm3, xmm0
    mov         eax, 3EF637C0h
    movd        xmm4, eax
    addss       xmm3, xmm4
    mulss       xmm3, xmm0
    mov         eax, 0BF38AA3Bh
    movd        xmm4, eax
    addss       xmm3, xmm4
    mulss       xmm3, xmm0
    mov         eax, 3FB8AA3Bh
    movd        xmm4, eax
    addss       xmm3, xmm4
    mulss       xmm3, xmm0
    addss       xmm3, xmm2
    mulss       xmm3, xmm1
    roundss     xmm4, xmm3, 1
    cvttss2si   eax, xmm4
    subss       xmm3, xmm4
    mov         ecx, 3AAF76A1h
    movd        xmm5, ecx
    mulss       xmm5, xmm3
    mov         ecx, 3C1D9539h
    movd        xmm4, ecx
    addss       xmm5, xmm4
    mulss       xmm5, xmm3
    mov         ecx, 3D635847h
    movd        xmm4, ecx
    addss       xmm5, xmm4
    mulss       xmm5, xmm3
    mov         ecx, 3E75FDF0h
    movd        xmm4, ecx
    addss       xmm5, xmm4
    mulss       xmm5, xmm3
    mov         ecx, 3F317218h
    movd        xmm4, ecx
    addss       xmm5, xmm4
    mulss       xmm5, xmm3
    mov         ecx, 3F800000h
    movd        xmm4, ecx
    addss       xmm5, xmm4
    add         eax, 127
    shl         eax, 23
    movd        xmm0, eax
    mulss       xmm0, xmm5
    ret

global SSEExpF32
align 16
SSEExpF32:
    mov         eax, 3FB8AA3Bh
    movd        xmm1, eax
    mulss       xmm0, xmm1
    roundss     xmm1, xmm0, 0
    cvttss2si   eax, xmm1
    subss       xmm0, xmm1
    mov         ecx, 3AAF047Bh
    movd        xmm1, ecx
    mulss       xmm1, xmm0
    mov         ecx, 3C1D9539h
    movd        xmm2, ecx
    addss       xmm1, xmm2
    mulss       xmm1, xmm0
    mov         ecx, 3D635847h
    movd        xmm2, ecx
    addss       xmm1, xmm2
    mulss       xmm1, xmm0
    mov         ecx, 3E75FDF0h
    movd        xmm2, ecx
    addss       xmm1, xmm2
    mulss       xmm1, xmm0
    mov         ecx, 3F317218h
    movd        xmm2, ecx
    addss       xmm1, xmm2
    mulss       xmm1, xmm0
    mov         ecx, 3F800000h
    movd        xmm2, ecx
    addss       xmm1, xmm2
    add         eax, 127
    shl         eax, 23
    movd        xmm0, eax
    mulss       xmm0, xmm1
    ret

global SSELnF32
align 16
SSELnF32:
    movd        eax, xmm0
    mov         ecx, eax
    shr         eax, 23
    sub         eax, 127
    cvtsi2ss    xmm1, eax
    and         ecx, 007FFFFFh
    or          ecx, 3F800000h
    movd        xmm0, ecx
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm0, xmm2
    movaps      xmm3, xmm0
    mulss       xmm3, xmm0
    mov         eax, 3E93DD98h
    movd        xmm4, eax
    mulss       xmm4, xmm0
    mov         eax, 0BE98AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3EF6384Fh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 0BF38AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3FB8AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    addss       xmm1, xmm4
    mov         eax, 3F317218h
    movd        xmm0, eax
    mulss       xmm0, xmm1
    ret

global SSELog10F32
align 16
SSELog10F32:
    movd        eax, xmm0
    mov         ecx, eax
    shr         eax, 23
    sub         eax, 127
    cvtsi2ss    xmm1, eax
    and         ecx, 007FFFFFh
    or          ecx, 3F800000h
    movd        xmm0, ecx
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm0, xmm2
    mov         eax, 3E93DD98h
    movd        xmm4, eax
    mulss       xmm4, xmm0
    mov         eax, 0BE98AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3EF6384Fh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 0BF38AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3FB8AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    addss       xmm1, xmm4
    mov         eax, 3E9A209Bh
    movd        xmm0, eax
    mulss       xmm0, xmm1
    ret

global SSELog2F32
align 16
SSELog2F32:
    movd        eax, xmm0
    mov         ecx, eax
    shr         eax, 23
    sub         eax, 127
    cvtsi2ss    xmm1, eax
    and         ecx, 007FFFFFh
    or          ecx, 3F800000h
    movd        xmm0, ecx
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm0, xmm2
    mov         eax, 3E93DD98h
    movd        xmm4, eax
    mulss       xmm4, xmm0
    mov         eax, 0BE98AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3EF6384Fh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 0BF38AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 3FB8AA3Bh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    addss       xmm1, xmm4
    movaps      xmm0, xmm1
    ret

global SSESinF32
align 16
SSESinF32:
    movd        eax, xmm0
    mov         ecx, eax
    and         ecx, 80000000h
    and         eax, 7FFFFFFFh
    movd        xmm0, eax
    mov         eax, 3F22F983h
    movd        xmm1, eax
    mulss       xmm1, xmm0
    roundss     xmm2, xmm1, 0
    cvtss2si    edx, xmm2
    mov         eax, 3FC90FDBh
    movd        xmm3, eax
    mulss       xmm3, xmm2
    subss       xmm0, xmm3
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 0B9500D01h
    movd        xmm4, eax
    mulss       xmm4, xmm1
    mov         eax, 3C088889h
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm1
    mov         eax, 0BE2AAAABh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm1
    mov         eax, 3F800000h
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 0BAB60B61h
    movd        xmm6, eax
    mulss       xmm6, xmm1
    mov         eax, 3D2AAAABh
    movd        xmm7, eax
    addss       xmm6, xmm7
    mulss       xmm6, xmm1
    mov         eax, 0BF000000h
    movd        xmm7, eax
    addss       xmm6, xmm7
    mulss       xmm6, xmm1
    mov         eax, 3F800000h
    movd        xmm7, eax
    addss       xmm6, xmm7
    and         edx, 3
    cmp         edx, 0
    je      @sin_q0
    cmp         edx, 1
    je      @sin_q1
    cmp         edx, 2
    je      @sin_q2
    mov         eax, 80000000h
    movd        xmm0, eax
    xorps       xmm6, xmm0
    movaps      xmm4, xmm6
    jmp     @sin_done

@sin_q0:
    jmp     @sin_done

@sin_q1:
    movaps      xmm4, xmm6
    jmp     @sin_done

@sin_q2:
    mov         eax, 80000000h
    movd        xmm0, eax
    xorps       xmm4, xmm0
    
@sin_done:
    movd        xmm0, ecx
    xorps       xmm0, xmm4
    ret

align 16
SSECosF32:
    movd        eax, xmm0
    and         eax, 7FFFFFFFh
    movd        xmm0, eax
    mov         eax, 3F22F983h
    movd        xmm1, eax
    mulss       xmm1, xmm0
    roundss     xmm2, xmm1, 0
    cvtss2si    edx, xmm2
    mov         eax, 3FC90FDBh
    movd        xmm3, eax
    mulss       xmm3, xmm2
    subss       xmm0, xmm3
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 0B9500D01h
    movd        xmm4, eax
    mulss       xmm4, xmm1
    mov         eax, 3C088889h
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm1
    mov         eax, 0BE2AAAABh
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm1
    mov         eax, 3F800000h
    movd        xmm5, eax
    addss       xmm4, xmm5
    mulss       xmm4, xmm0
    mov         eax, 0BAB60B61h
    movd        xmm6, eax
    mulss       xmm6, xmm1
    mov         eax, 3D2AAAABh
    movd        xmm7, eax
    addss       xmm6, xmm7
    mulss       xmm6, xmm1
    mov         eax, 0BF000000h
    movd        xmm7, eax
    addss       xmm6, xmm7
    mulss       xmm6, xmm1
    mov         eax, 3F800000h
    movd        xmm7, eax
    addss       xmm6, xmm7
    and         edx, 3
    cmp         edx, 0
    je      @cos_q0
    cmp         edx, 1
    je      @cos_q1
    cmp         edx, 2
    je      @cos_q2
    movaps      xmm0, xmm4
    ret

@cos_q0:
    movaps      xmm0, xmm6
    ret

@cos_q1:
    mov         eax, 80000000h
    movd        xmm0, eax
    xorps       xmm0, xmm4
    ret

@cos_q2:
    mov         eax, 80000000h
    movd        xmm0, eax
    xorps       xmm0, xmm6
    ret

global SSETanF32
align 16
SSETanF32:
    push        rbx
    sub         rsp, 32
    movss       dword [rsp], xmm0
    call    SSESinF32
    movss       dword [rsp - 16], xmm0
    movss       xmm0, dword [rsp]
    call    SSECosF32
    movss       xmm1, dword [rsp - 16]
    divss       xmm1, xmm0
    movaps      xmm0, xmm1
    add         rsp, 32
    pop         rbx
    ret

global SSEArcSinF32
align 16
SSEArcSinF32:
    movd        eax, xmm0
    mov         ecx, eax
    and         ecx, 80000000h
    and         eax, 7FFFFFFFh
    movd        xmm0, eax
    mov         eax, 3F000000h
    movd        xmm1, eax
    comiss      xmm0, xmm1
    ja      @asin_large
    movaps      xmm2, xmm0
    mulss       xmm0, xmm0
    mov         eax, 3CB8B41Ah
    movd        xmm1, eax
    mulss       xmm1, xmm0
    mov         eax, 3D36DB6Eh
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm0
    mov         eax, 3E2AAAABh
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm0
    mov         eax, 3F800000h
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm2
    movaps      xmm0, xmm1
    jmp     @asin_apply_sign
    
@asin_large:
    sub         rsp, 16
    mov         [rsp], ecx
    mov         eax, 3F800000h
    movd        xmm1, eax
    subss       xmm1, xmm0
    mov         eax, 3F000000h
    movd        xmm2, eax
    mulss       xmm1, xmm2
    sqrtss      xmm0, xmm1
    movaps      xmm2, xmm0
    mulss       xmm0, xmm0
    mov         eax, 3CB8B41Ah
    movd        xmm1, eax
    mulss       xmm1, xmm0
    mov         eax, 3D36DB6Eh
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm0
    mov         eax, 3E2AAAABh
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm0
    mov         eax, 3F800000h
    movd        xmm3, eax
    addss       xmm1, xmm3
    mulss       xmm1, xmm2
    addss       xmm1, xmm1
    mov         eax, 3FC90FDBh
    movd        xmm0, eax
    subss       xmm0, xmm1
    mov         ecx, [rsp]
    add         rsp, 16
    
@asin_apply_sign:
    movd        xmm1, ecx
    xorps       xmm0, xmm1
    ret

global SSEArcCosF32
align 16
SSEArcCosF32:
    call    SSEArcSinF32
    mov         eax, 3FC90FDBh
    movd        xmm1, eax
    subss       xmm1, xmm0
    movaps      xmm0, xmm1
    ret

global SSEArcTanF32
align 16
SSEArcTanF32:
    movd        eax, xmm0
    mov         ecx, eax
    and         ecx, 80000000h
    and         eax, 7FFFFFFFh
    movd        xmm0, eax
    mov         eax, 3F800000h
    movd        xmm1, eax
    comiss      xmm0, xmm1
    ja      @atan_large
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 3DE38E39h
    movd        xmm2, eax
    mulss       xmm2, xmm1
    mov         eax, 3E124925h
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3E4CCCCDh
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm2, xmm1
    mov         eax, 3EAAAAABh
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm0, xmm2
    jmp     @atan_done
    
@atan_large:
    mov         eax, 3F800000h
    movd        xmm1, eax
    divss       xmm1, xmm0
    movaps      xmm0, xmm1
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 3DE38E39h
    movd        xmm2, eax
    mulss       xmm2, xmm1
    mov         eax, 3E124925h
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3E4CCCCDh
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm2, xmm1
    mov         eax, 3EAAAAABh
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm0, xmm2
    mov         eax, 3FC90FDBh
    movd        xmm1, eax
    subss       xmm1, xmm0
    movaps      xmm0, xmm1
    
@atan_done:
    movd        xmm1, ecx
    xorps       xmm0, xmm1
    ret

global SSEArcTan2F32
align 16
SSEArcTan2F32:
    sub         rsp, 32
    movd        eax, xmm0
    movd        edx, xmm1
    mov         ecx, eax
    and         ecx, 80000000h
    mov         [rsp], ecx
    mov         r8d, edx
    and         r8d, 80000000h
    mov         [rsp - 16], r8d
    and         eax, 7FFFFFFFh
    and         edx, 7FFFFFFFh
    movd        xmm0, eax
    movd        xmm1, edx
    test        edx, edx
    jz      @atan2_xzero
    divss       xmm0, xmm1
    movaps      xmm2, xmm0
    mov         eax, 3F800000h
    movd        xmm1, eax
    comiss      xmm0, xmm1
    ja      @atan2_ratio_large
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 3DE38E39h
    movd        xmm2, eax
    mulss       xmm2, xmm1
    mov         eax, 3E124925h
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3E4CCCCDh
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm2, xmm1
    mov         eax, 3EAAAAABh
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm0, xmm2
    jmp     @atan2_quadrant
    
@atan2_ratio_large:
    mov         eax, 3F800000h
    movd        xmm1, eax
    divss       xmm1, xmm0
    movaps      xmm0, xmm1
    movaps      xmm1, xmm0
    mulss       xmm1, xmm0
    mov         eax, 3DE38E39h
    movd        xmm2, eax
    mulss       xmm2, xmm1
    mov         eax, 3E124925h
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3E4CCCCDh
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm2, xmm1
    mov         eax, 3EAAAAABh
    movd        xmm3, eax
    subss       xmm3, xmm2
    mulss       xmm3, xmm1
    mov         eax, 3F800000h
    movd        xmm2, eax
    subss       xmm2, xmm3
    mulss       xmm0, xmm2
    mov         eax, 3FC90FDBh
    movd        xmm1, eax
    subss       xmm1, xmm0
    movaps      xmm0, xmm1
    
@atan2_quadrant:
    mov         ecx, [rsp]
    mov         r8d, [rsp - 16]
    test        r8d, r8d
    jnz     @atan2_xneg
    movd        xmm1, ecx
    xorps       xmm0, xmm1
    jmp     @atan2_done
    
@atan2_xneg:
    mov         eax, 40490FDBh
    movd        xmm1, eax
    test        ecx, ecx
    jnz     @atan2_xneg_yneg
    subss       xmm1, xmm0
    movaps      xmm0, xmm1
    jmp     @atan2_done
    
@atan2_xneg_yneg:
    subss       xmm0, xmm1
    jmp     @atan2_done
    
@atan2_xzero:
    mov         eax, 3FC90FDBh
    movd        xmm0, eax
    mov         ecx, [rsp]
    movd        xmm1, ecx
    xorps       xmm0, xmm1
    
@atan2_done:
    add         rsp, 32
    ret

;; 64-bit______________________________________________________________________
global SSEInvSqrtF64
align 16
SSEInvSqrtF64:
    sqrtsd      xmm1, xmm0
    mov         rax, 3FF0000000000000h
    movq        xmm0, rax
    divsd       xmm0, xmm1
    ret

global SSEPowF64 
align 16
SSEPowF64:
    sub         rsp, 32
    movsd       qword [rsp - 16], xmm1
    call    SSELnF64
    mulsd       xmm0, qword [rsp - 16]
    call    SSEExpF64
    add         rsp, 32
    ret

global SSEExpF64
align 16
SSEExpF64:
    mov         rax, 3FF71547652B82FEh
    movq        xmm1, rax
    mulsd       xmm0, xmm1
    roundsd     xmm1, xmm0, 0
    cvttsd2si   rax, xmm1
    subsd       xmm0, xmm1
    mov         rcx, 3E5ADE156A5DCB37h
    movq        xmm2, rcx
    mulsd       xmm2, xmm0
    mov         rcx, 3E928AF3FCA213EAh
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3EC71DEE623FDE64h
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3EFA01997C89E6B0h
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3F2A01A014761F6Eh
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3F55547652B82FEFh
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3FE62E42FEFA39EFh
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm0
    mov         rcx, 3FF0000000000000h
    movq        xmm3, rcx
    addsd       xmm2, xmm3
    add         rax, 1023
    shl         rax, 52
    movq        xmm1, rax
    mulsd       xmm2, xmm1
    movapd      xmm0, xmm2
    ret

global SSELnF64
align 16
SSELnF64:
    movq        rax, xmm0
    mov         rcx, rax
    shr         rax, 52
    sub         eax, 1023
    cvtsi2sd    xmm1, eax
    mov         rax, 000FFFFFFFFFFFFFh
    and         rcx, rax
    mov         rax, 3FF0000000000000h
    or          rcx, rax
    movq        xmm0, rcx
    mov         rax, 3FF0000000000000h
    movq        xmm2, rax
    subsd       xmm0, xmm2
    mov         rax, 3CAF67C5B70C0634h
    movq        xmm3, rax
    mulsd       xmm3, xmm0
    mov         rax, 3CD1A763E9D95D00h
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 0BD30D555AE4A500h
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FD2776C50EF9BFEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 0BFD71547652B82FEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FDEC709DC3A03FDh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FF71547652B82FEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    addsd       xmm1, xmm3
    mov         rax, 3FE62E42FEFA39EFh
    movq        xmm0, rax
    mulsd       xmm0, xmm1
    ret

global SSELog10F64
align 16
SSELog10F64:
    call    SSELnF64
    mov         rax, 3FDBCB7B1526E50Eh
    movq        xmm1, rax
    mulsd       xmm0, xmm1
    ret

global SSELog2F64
align 16
SSELog2F64:
    movq        rax, xmm0
    mov         rcx, rax
    shr         rax, 52
    sub         eax, 1023
    cvtsi2sd    xmm1, eax
    mov         rax, 000FFFFFFFFFFFFFh
    and         rcx, rax
    mov         rax, 3FF0000000000000h
    or          rcx, rax
    movq        xmm0, rcx
    mov         rax, 3FF0000000000000h
    movq        xmm2, rax
    subsd       xmm0, xmm2
    mov         rax, 3CAF67C5B70C0634h
    movq        xmm3, rax
    mulsd       xmm3, xmm0
    mov         rax, 3CD1A763E9D95D00h
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 0BD30D555AE4A500h
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FD2776C50EF9BFEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 0BFD71547652B82FEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FDEC709DC3A03FDh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    mov         rax, 3FF71547652B82FEh
    movq        xmm4, rax
    addsd       xmm3, xmm4
    mulsd       xmm3, xmm0
    addsd       xmm1, xmm3
    movapd      xmm0, xmm1
    ret

global SSESinF64
align 16
SSESinF64:
    movq        rax, xmm0
    mov         rcx, rax
    mov         r11, 80000000h
    and         rcx, r11
    mov         [rsp - 8], rcx
    and         rax, 7FFFFFFFh
    movq        xmm0, rax
    mov         rax, 3FE45F306DC9C883h
    movq        xmm1, rax
    mulsd       xmm1, xmm0
    roundsd     xmm2, xmm1, 0
    cvttsd2si   edx, xmm2
    mov         rax, 3FF921FB54442D18h
    movq        xmm3, rax
    mulsd       xmm3, xmm2
    subsd       xmm0, xmm3
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 0BE5AE5E68A2B9CEBh
    movq        xmm4, rax
    mulsd       xmm4, xmm1
    mov         rax, 3EC71DE3A556C734h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 0BF2A01A019C161D5h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 3F8111111110F8A6h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 0BFC5555555555549h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm0
    mov         rax, 3E21EE9EBDB4B1C4h
    movq        xmm6, rax
    mulsd       xmm6, xmm1
    mov         rax, 0BE927E4F809C52ADh
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 3EFA01A019CB1590h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 0BF56C16C16C15177h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    and         edx, 3
    cmp         edx, 0
    je      @sin64_q0
    cmp         edx, 1
    je      @sin64_q1
    cmp         edx, 2
    je      @sin64_q2
    mov         rax, 80000000h
    movq        xmm0, rax
    xorpd       xmm4, xmm0
    movapd      xmm4, xmm6
    mov         rax, 80000000h
    movq        xmm0, rax
    xorpd       xmm4, xmm0
    jmp     @sin64_done

@sin64_q0:
    jmp     @sin64_done

@sin64_q1:
    movapd      xmm4, xmm6
    jmp     @sin64_done

@sin64_q2:
    mov         rax, 80000000h
    movq        xmm0, rax
    xorpd       xmm4, xmm0
    
@sin64_done:
    mov         rcx, [rsp - 8]
    movq        xmm0, rcx
    xorpd       xmm0, xmm4
    ret

global SSECosF64
align 16
SSECosF64:
    movq        rax, xmm0
    mov         rcx, 7FFFFFFFFFFFFFFFh
    and         rax, rcx
    movq        xmm0, rax
    mov         rax, 3FE45F306DC9C883h
    movq        xmm1, rax
    mulsd       xmm1, xmm0
    roundsd     xmm2, xmm1, 0
    cvttsd2si   edx, xmm2
    mov         rax, 3FF921FB54442D18h
    movq        xmm3, rax
    mulsd       xmm3, xmm2
    subsd       xmm0, xmm3
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 0BE5AE5E68A2B9CEBh
    movq        xmm4, rax
    mulsd       xmm4, xmm1
    mov         rax, 3EC71DE3A556C734h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 0BF2A01A019C161D5h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 3F8111111110F8A6h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 0BFC5555555555549h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm5, rax
    addsd       xmm4, xmm5
    mulsd       xmm4, xmm0
    mov         rax, 3E21EE9EBDB4B1C4h
    movq        xmm6, rax
    mulsd       xmm6, xmm1
    mov         rax, 0BE927E4F809C52ADh
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 3EFA01A019CB1590h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 0BF56C16C16C15177h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    mulsd       xmm6, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm7, rax
    addsd       xmm6, xmm7
    and         edx, 3
    cmp         edx, 0
    je      @cos64_q0
    cmp         edx, 1
    je      @cos64_q1
    cmp         edx, 2
    je      @cos64_q2
    movapd      xmm0, xmm4
    ret

@cos64_q0:
    movapd      xmm0, xmm6
    ret

@cos64_q1:
    mov         rax, 8000000000000000h
    movq        xmm0, rax
    xorpd       xmm0, xmm4
    ret

@cos64_q2:
    mov         rax, 8000000000000000h
    movq        xmm0, rax
    xorpd       xmm0, xmm6
    ret

global SSETanF64
align 16
SSETanF64:
    sub         rsp, 32
    movsd       qword [rsp], xmm0
    call    SSESinF64
    movsd       qword [rsp - 16], xmm0
    movsd       xmm0, qword [rsp]
    call    SSECosF64
    movsd       xmm1, qword [rsp - 16]
    divsd       xmm1, xmm0
    movapd      xmm0, xmm1
    add         rsp, 32
    ret

global SSEArcSinF64
align 16
SSEArcSinF64:
    movq        rax, xmm0
    mov         rcx, rax
    mov         r11, 8000000000000000h
    and         rcx, r11
    mov         r11, 7FFFFFFFFFFFFFFFh
    and         rax, r11
    movq        xmm0, rax
    mov         rax, 3FE0000000000000h
    movq        xmm1, rax
    comisd      xmm0, xmm1
    ja      @asin64_large
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 3F9F1C72E13AD8BEh
    movq        xmm2, rax
    mulsd       xmm2, xmm1
    mov         rax, 3FA6DB6DB3B44600h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FB3333333331C55h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FC5555555555555h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm0, xmm2
    jmp     @asin64_done
    
@asin64_large:
    sub         rsp, 16
    mov         [rsp - 8], rcx
    mov         rax, 3FF0000000000000h
    movq        xmm1, rax
    subsd       xmm1, xmm0
    mov         rax, 3FE0000000000000h
    movq        xmm2, rax
    mulsd       xmm1, xmm2
    sqrtsd      xmm0, xmm1
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 3F9F1C72E13AD8BEh
    movq        xmm2, rax
    mulsd       xmm2, xmm1
    mov         rax, 3FA6DB6DB3B44600h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FB3333333331C55h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FC5555555555555h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm3, rax
    addsd       xmm2, xmm3
    mulsd       xmm0, xmm2
    addsd       xmm0, xmm0
    mov         rax, 3FF921FB54442D18h
    movq        xmm1, rax
    subsd       xmm0, xmm1
    mov         rax, 8000000000000000h
    movq        xmm2, rax
    xorpd       xmm0, xmm2
    mov         rcx, [rsp - 8]
    add         rsp, 16
    
@asin64_done:
    movq        xmm1, rcx
    xorpd       xmm0, xmm1
    ret

global SSEArcCosF64
align 16
SSEArcCosF64:
    call    SSEArcSinF64
    mov         rax, 3FF921FB54442D18h
    movq        xmm1, rax
    subsd       xmm1, xmm0
    movapd      xmm0, xmm1
    ret

global SSEArcTanF64
align 16
SSEArcTanF64:
    movq        rax, xmm0
    mov         rcx, rax
    mov         r11, 8000000000000000h
    and         rcx, r11
    mov         r11, 7FFFFFFFFFFFFFFFh
    and         rax, r11
    movq        xmm0, rax
    mov         rax, 3FF0000000000000h
    movq        xmm1, rax
    comisd      xmm0, xmm1
    ja      @atan64_large
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 3FB745D171BF675Dh
    movq        xmm2, rax
    mulsd       xmm2, xmm1
    mov         rax, 3FBC71C6F5B4C600h
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm3, xmm1
    mov         rax, 3FC2492492492492h
    movq        xmm2, rax
    subsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FC999999999999Ah
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm3, xmm1
    mov         rax, 3FD5555555555555h
    movq        xmm2, rax
    subsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm0, xmm3
    jmp     @atan64_done
    
@atan64_large:
    mov         rax, 3FF0000000000000h
    movq        xmm1, rax
    divsd       xmm1, xmm0
    movapd      xmm0, xmm1
    movapd      xmm1, xmm0
    mulsd       xmm1, xmm0
    mov         rax, 3FB745D171BF675Dh
    movq        xmm2, rax
    mulsd       xmm2, xmm1
    mov         rax, 3FBC71C6F5B4C600h
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm3, xmm1
    mov         rax, 3FC2492492492492h
    movq        xmm2, rax
    subsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FC999999999999Ah
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm3, xmm1
    mov         rax, 3FD5555555555555h
    movq        xmm2, rax
    subsd       xmm2, xmm3
    mulsd       xmm2, xmm1
    mov         rax, 3FF0000000000000h
    movq        xmm3, rax
    subsd       xmm3, xmm2
    mulsd       xmm0, xmm3
    mov         rax, 3FF921FB54442D18h
    movq        xmm1, rax
    subsd       xmm1, xmm0
    movapd      xmm0, xmm1
    
@atan64_done:
    movq        xmm1, rcx
    xorpd       xmm0, xmm1
    ret

global SSEArcTan2F64
align 16
SSEArcTan2F64:
    sub         rsp, 32
    movq        rax, xmm0
    mov         rcx, rax
    mov         r11, 8000000000000000h
    and         rcx, r11
    mov         [rsp], rcx
    movq        rdx, xmm1
    mov         r8, rdx
    and         r8, r11
    mov         [rsp - 8], r8
    mov         r11, 7FFFFFFFFFFFFFFFh
    and         rax, r11
    and         rdx, r11
    movq        xmm0, rax
    movq        xmm1, rdx
    test        rdx, rdx
    jz      @atan264_xzero
    divsd       xmm0, xmm1
    movsd       qword [rsp - 16], xmm0
    call    SSEArcTanF64
    mov         rcx, [rsp]
    mov         r8, [rsp - 8]
    test        r8, r8
    jz          @atan264_xpos
    mov         rax, 400921FB54442D18h
    movq        xmm1, rax
    test        rcx, rcx
    jz          @atan264_xneg_ypos
    subsd       xmm0, xmm1
    jmp         @atan264_done

@atan264_xneg_ypos:
    mov         rax, 8000000000000000h
    movq        xmm2, rax
    xorpd       xmm0, xmm2
    addsd       xmm0, xmm1
    jmp         @atan264_done
    
@atan264_xpos:
    movq        xmm1, rcx
    xorpd       xmm0, xmm1
    jmp         @atan264_done
    
@atan264_xzero:
    mov         rax, 3FF921FB54442D18h
    movq        xmm0, rax
    mov         rcx, [rsp]
    movq        xmm1, rcx
    xorpd       xmm0, xmm1
    
@atan264_done:
    add         rsp, 32
    ret


;;
;; AVX
;; ____________________________________________________________________________
;;

global AVXPowF32
align 16
AVXPowF32:
    sub             rsp, 16
    vmovss          dword [rsp + 8], xmm1
    call            AVXLnF32
    vmulss          xmm0, xmm0, dword [rsp + 8]
    call            AVXExpF32
    add             rsp, 16
    ret

global AVXExpF32
align 16
AVXExpF32:
    mov             eax, 3FB8AA3Bh
    vmovd           xmm1, eax
    vmulss          xmm0, xmm0, xmm1
    vroundss        xmm1, xmm0, xmm0, 0
    vcvttss2si      eax, xmm1
    vsubss          xmm0, xmm0, xmm1
    mov             ecx, 3AAF047Bh
    vmovd           xmm1, ecx
    mov             ecx, 3C1D9539h
    vmovd           xmm2, ecx
    vfmadd213ss     xmm1, xmm0, xmm2
    mov             ecx, 3D635847h
    vmovd           xmm2, ecx
    vfmadd213ss     xmm1, xmm0, xmm2
    mov             ecx, 3E75FDF0h
    vmovd           xmm2, ecx
    vfmadd213ss     xmm1, xmm0, xmm2
    mov             ecx, 3F317218h
    vmovd           xmm2, ecx
    vfmadd213ss     xmm1, xmm0, xmm2
    mov             ecx, 3F800000h
    vmovd           xmm2, ecx
    vfmadd213ss     xmm1, xmm0, xmm2
    add             eax, 127
    shl             eax, 23
    vmovd           xmm0, eax
    vmulss          xmm0, xmm0, xmm1
    ret

global AVXLnF32
align 16
AVXLnF32:
    vmovd           eax, xmm0
    mov             ecx, eax
    shr             eax, 23
    sub             eax, 127
    vcvtsi2ss       xmm1, xmm1, eax
    and             ecx, 007FFFFFh
    or              ecx, 3F800000h
    vmovd           xmm0, ecx
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm0, xmm0, xmm2
    mov             eax, 3E93DD98h
    vmovd           xmm4, eax
    mov             ecx, 0BE98AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3EF6384Fh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 0BF38AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3FB8AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    vmulss          xmm4, xmm4, xmm0
    vaddss          xmm1, xmm1, xmm4
    mov             eax, 3F317218h
    vmovd           xmm0, eax
    vmulss          xmm0, xmm0, xmm1
    ret

global AVXLog10F32
align 16
AVXLog10F32:
    vmovd           eax, xmm0
    mov             ecx, eax
    shr             eax, 23
    sub             eax, 127
    vcvtsi2ss       xmm1, xmm1, eax
    and             ecx, 007FFFFFh
    or              ecx, 3F800000h
    vmovd           xmm0, ecx
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm0, xmm0, xmm2
    mov             eax, 3E93DD98h
    vmovd           xmm4, eax
    mov             ecx, 0BE98AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3EF6384Fh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 0BF38AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3FB8AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    vmulss          xmm4, xmm4, xmm0
    vaddss          xmm1, xmm1, xmm4
    mov             eax, 3E9A209Bh
    vmovd           xmm0, eax
    vmulss          xmm0, xmm0, xmm1
    ret

global AVXLog2F32
align 16
AVXLog2F32:
    vmovd           eax, xmm0
    mov             ecx, eax
    shr             eax, 23
    sub             eax, 127
    vcvtsi2ss       xmm1, xmm1, eax
    and             ecx, 007FFFFFh
    or              ecx, 3F800000h
    vmovd           xmm0, ecx
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm0, xmm0, xmm2
    mov             eax, 3E93DD98h
    vmovd           xmm4, eax
    mov             ecx, 0BE98AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3EF6384Fh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 0BF38AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    mov             ecx, 3FB8AA3Bh
    vmovd           xmm5, ecx
    vfmadd213ss     xmm4, xmm0, xmm5
    vmulss          xmm4, xmm4, xmm0
    vaddss          xmm0, xmm1, xmm4
    ret

global AVXSinF32
align 16
AVXSinF32:
    vmovd           eax, xmm0
    mov             ecx, eax
    and             ecx, 80000000h
    and             eax, 7FFFFFFFh
    vmovd           xmm0, eax
    mov             eax, 3F22F983h
    vmovd           xmm1, eax
    vmulss          xmm1, xmm1, xmm0
    vroundss        xmm2, xmm1, xmm1, 0
    vcvtss2si       edx, xmm2
    mov             eax, 3FC90FDBh
    vmovd           xmm3, eax
    vmulss          xmm3, xmm3, xmm2
    vsubss          xmm0, xmm0, xmm3
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 0B9500D01h
    vmovd           xmm4, eax
    mov             eax, 3C088889h
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    mov             eax, 0BE2AAAABh
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    mov             eax, 3F800000h
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    vmulss          xmm4, xmm4, xmm0
    mov             eax, 0BAB60B61h
    vmovd           xmm6, eax
    mov             eax, 3D2AAAABh
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    mov             eax, 0BF000000h
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    mov             eax, 3F800000h
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    and             edx, 3
    cmp             edx, 0
    je          @avxsin_q0
    cmp             edx, 1
    je          @avxsin_q1
    cmp             edx, 2
    je          @avxsin_q2
    mov             eax, 80000000h
    vmovd           xmm0, eax
    vxorps          xmm4, xmm6, xmm0
    jmp         @avxsin_done

@avxsin_q0:
    jmp         @avxsin_done

@avxsin_q1:
    vmovaps         xmm4, xmm6
    jmp         @avxsin_done

@avxsin_q2:
    mov             eax, 80000000h
    vmovd           xmm0, eax
    vxorps          xmm4, xmm4, xmm0
    
@avxsin_done:
    vmovd           xmm0, ecx
    vxorps          xmm0, xmm0, xmm4
    ret

global AVXCosF32
align 16
AVXCosF32:
    vmovd           eax, xmm0
    and             eax, 7FFFFFFFh
    vmovd           xmm0, eax
    mov             eax, 3F22F983h
    vmovd           xmm1, eax
    vmulss          xmm1, xmm1, xmm0
    vroundss        xmm2, xmm1, xmm1, 0
    vcvtss2si       edx, xmm2
    mov             eax, 3FC90FDBh
    vmovd           xmm3, eax
    vmulss          xmm3, xmm3, xmm2
    vsubss          xmm0, xmm0, xmm3
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 0B9500D01h
    vmovd           xmm4, eax
    mov             eax, 3C088889h
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    mov             eax, 0BE2AAAABh
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    mov             eax, 3F800000h
    vmovd           xmm5, eax
    vfmadd213ss     xmm4, xmm1, xmm5
    vmulss          xmm4, xmm4, xmm0
    mov             eax, 0BAB60B61h
    vmovd           xmm6, eax
    mov             eax, 3D2AAAABh
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    mov             eax, 0BF000000h
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    mov             eax, 3F800000h
    vmovd           xmm7, eax
    vfmadd213ss     xmm6, xmm1, xmm7
    and             edx, 3
    cmp             edx, 0
    je          @avxcos_q0
    cmp             edx, 1
    je          @avxcos_q1
    cmp             edx, 2
    je          @avxcos_q2
    vmovaps         xmm0, xmm4
    ret

@avxcos_q0:
    vmovaps         xmm0, xmm6
    ret

@avxcos_q1:
    mov             eax, 80000000h
    vmovd           xmm0, eax
    vxorps          xmm0, xmm0, xmm4
    ret

@avxcos_q2:
    mov             eax, 80000000h
    vmovd           xmm0, eax
    vxorps          xmm0, xmm0, xmm6
    ret

global AVXTanF32
align 16
AVXTanF32:
    sub             rsp, 32
    vmovss          dword [rsp], xmm0
    call        AVXSinF32
    vmovss          dword [rsp - 16], xmm0
    vmovss          xmm0, dword [rsp]
    call        AVXCosF32
    vmovss          xmm1, dword [rsp - 16]
    vdivss          xmm0, xmm1, xmm0
    add             rsp, 32
    ret

global AVXArcSinF32
align 16
AVXArcSinF32:
    vmovd           eax, xmm0
    mov             ecx, eax
    and             ecx, 80000000h
    and             eax, 7FFFFFFFh
    vmovd           xmm0, eax
    mov             eax, 3F000000h
    vmovd           xmm1, eax
    vcomiss         xmm0, xmm1
    ja          @avxasin_large
    vmovaps         xmm2, xmm0
    vmulss          xmm0, xmm0, xmm0
    mov             eax, 3CB8B41Ah
    vmovd           xmm1, eax
    mov             eax, 3D36DB6Eh
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    mov             eax, 3E2AAAABh
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    mov             eax, 3F800000h
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    vmulss          xmm0, xmm1, xmm2
    jmp         @avxasin_apply_sign
    
@avxasin_large:
    sub             rsp, 16
    mov             [rsp], ecx
    mov             eax, 3F800000h
    vmovd           xmm1, eax
    vsubss          xmm1, xmm1, xmm0
    mov             eax, 3F000000h
    vmovd           xmm2, eax
    vmulss          xmm1, xmm1, xmm2
    vsqrtss         xmm0, xmm0, xmm1
    vmovaps         xmm2, xmm0
    vmulss          xmm0, xmm0, xmm0
    mov             eax, 3CB8B41Ah
    vmovd           xmm1, eax
    mov             eax, 3D36DB6Eh
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    mov             eax, 3E2AAAABh
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    mov             eax, 3F800000h
    vmovd           xmm3, eax
    vfmadd213ss     xmm1, xmm0, xmm3
    vmulss          xmm1, xmm1, xmm2
    vaddss          xmm1, xmm1, xmm1
    mov             eax, 3FC90FDBh
    vmovd           xmm0, eax
    vsubss          xmm0, xmm0, xmm1
    mov             ecx, [rsp]
    add             rsp, 16
    
@avxasin_apply_sign:
    vmovd           xmm1, ecx
    vxorps          xmm0, xmm0, xmm1
    ret

global AVXArcCosF32
align 16
AVXArcCosF32:
    call        AVXArcSinF32
    mov             eax, 3FC90FDBh
    vmovd           xmm1, eax
    vsubss          xmm0, xmm1, xmm0
    ret

global AVXArcTanF32
align 16
AVXArcTanF32:
    vmovd           eax, xmm0
    mov             ecx, eax
    and             ecx, 80000000h
    and             eax, 7FFFFFFFh
    vmovd           xmm0, eax
    mov             eax, 3F800000h
    vmovd           xmm1, eax
    vcomiss         xmm0, xmm1
    ja          @avxatan_large
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 3DE38E39h
    vmovd           xmm2, eax
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3E124925h
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3E4CCCCDh
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3EAAAAABh
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm0, xmm0, xmm2
    jmp         @avxatan_done
    
@avxatan_large:
    mov             eax, 3F800000h
    vmovd           xmm1, eax
    vdivss          xmm0, xmm1, xmm0
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 3DE38E39h
    vmovd           xmm2, eax
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3E124925h
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3E4CCCCDh
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3EAAAAABh
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm0, xmm0, xmm2
    mov             eax, 3FC90FDBh
    vmovd           xmm1, eax
    vsubss          xmm0, xmm1, xmm0
    
@avxatan_done:
    vmovd           xmm1, ecx
    vxorps          xmm0, xmm0, xmm1
    ret

global AVXArcTan2F32
align 16
AVXArcTan2F32:
    sub             rsp, 32
    vmovd           eax, xmm0
    vmovd           edx, xmm1
    mov             ecx, eax
    and             ecx, 80000000h
    mov             [rsp], ecx
    mov             r8d, edx
    and             r8d, 80000000h
    mov             [rsp - 16], r8d
    and             eax, 7FFFFFFFh
    and             edx, 7FFFFFFFh
    vmovd           xmm0, eax
    vmovd           xmm1, edx
    test            edx, edx
    jz          @avxatan2_xzero
    vdivss          xmm0, xmm0, xmm1
    vmovaps         xmm2, xmm0
    mov             eax, 3F800000h
    vmovd           xmm1, eax
    vcomiss         xmm0, xmm1
    ja          @avxatan2_ratio_large
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 3DE38E39h
    vmovd           xmm2, eax
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3E124925h
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3E4CCCCDh
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3EAAAAABh
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm0, xmm0, xmm2
    jmp         @avxatan2_quadrant
    
@avxatan2_ratio_large:
    mov             eax, 3F800000h
    vmovd           xmm1, eax
    vdivss          xmm0, xmm1, xmm0
    vmulss          xmm1, xmm0, xmm0
    mov             eax, 3DE38E39h
    vmovd           xmm2, eax
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3E124925h
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3E4CCCCDh
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm2, xmm2, xmm1
    mov             eax, 3EAAAAABh
    vmovd           xmm3, eax
    vsubss          xmm3, xmm3, xmm2
    vmulss          xmm3, xmm3, xmm1
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vsubss          xmm2, xmm2, xmm3
    vmulss          xmm0, xmm0, xmm2
    mov             eax, 3FC90FDBh
    vmovd           xmm1, eax
    vsubss          xmm0, xmm1, xmm0
    
@avxatan2_quadrant:
    mov             ecx, [rsp]
    mov             r8d, [rsp - 16]
    test            r8d, r8d
    jnz         @avxatan2_xneg
    vmovd           xmm1, ecx
    vxorps          xmm0, xmm0, xmm1
    jmp         @avxatan2_done
    
@avxatan2_xneg:
    mov             eax, 40490FDBh
    vmovd           xmm1, eax
    test            ecx, ecx
    jnz         @avxatan2_xneg_yneg
    vsubss          xmm0, xmm1, xmm0
    jmp         @avxatan2_done
    
@avxatan2_xneg_yneg:
    vsubss          xmm0, xmm0, xmm1
    jmp         @avxatan2_done
    
@avxatan2_xzero:
    mov             eax, 3FC90FDBh
    vmovd           xmm0, eax
    mov             ecx, [rsp]
    vmovd           xmm1, ecx
    vxorps          xmm0, xmm0, xmm1
    
@avxatan2_done:
    add             rsp, 32
    ret

;; 64-bit______________________________________________________________________
global AVXInvSqrtF64
align 16
AVXInvSqrtF64:
    vsqrtsd         xmm1, xmm0, xmm0
    mov             rax, 3FF0000000000000h
    vmovq           xmm0, rax
    vdivsd          xmm0, xmm0, xmm1
    ret

global AVXPowF64
align 16
AVXPowF64:
    sub             rsp, 32
    vmovsd          qword [rsp - 16], xmm1
    call        AVXLnF64
    vmulsd          xmm0, xmm0, qword [rsp - 16]
    call        AVXExpF64
    add             rsp, 32
    ret

global AVXExpF64
align 16
AVXExpF64:
    mov             rax, 3FF71547652B82FEh
    vmovq           xmm1, rax
    vmulsd          xmm0, xmm0, xmm1
    vroundsd        xmm1, xmm0, xmm0, 0
    vcvttsd2si      rax, xmm1
    vsubsd          xmm0, xmm0, xmm1
    mov             rcx, 3E5ADE156A5DCB37h
    vmovq           xmm2, rcx
    mov             rcx, 3E928AF3FCA213EAh
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3EC71DEE623FDE64h
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3EFA01997C89E6B0h
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3F2A01A014761F6Eh
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3F55547652B82FEFh
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3FE62E42FEFA39EFh
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    mov             rcx, 3FF0000000000000h
    vmovq           xmm3, rcx
    vfmadd213sd     xmm2, xmm0, xmm3
    add             rax, 1023
    shl             rax, 52
    vmovq           xmm1, rax
    vmulsd          xmm0, xmm2, xmm1
    ret

global AVXLnF64
align 16
AVXLnF64:
    vmovq           rax, xmm0
    mov             rcx, rax
    shr             rax, 52
    sub             eax, 1023
    vcvtsi2sd       xmm1, xmm1, eax
    mov             rax, 000FFFFFFFFFFFFFh
    and             rcx, rax
    mov             rax, 3FF0000000000000h
    or              rcx, rax
    vmovq           xmm0, rcx
    mov             rax, 3FF0000000000000h
    vmovq           xmm2, rax
    vsubsd          xmm0, xmm0, xmm2
    mov             rax, 3CAF67C5B70C0634h
    vmovq           xmm3, rax
    mov             rax, 3CD1A763E9D95D00h
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 0BD30D555AE4A500h
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FD2776C50EF9BFEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 0BFD71547652B82FEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FDEC709DC3A03FDh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FF71547652B82FEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    vmulsd          xmm3, xmm3, xmm0
    vaddsd          xmm1, xmm1, xmm3
    mov             rax, 3FE62E42FEFA39EFh
    vmovq           xmm0, rax
    vmulsd          xmm0, xmm0, xmm1
    ret

global AVXLog10F64
align 16
AVXLog10F64:
    call        AVXLnF64
    mov             rax, 3FDBCB7B1526E50Eh
    vmovq           xmm1, rax
    vmulsd          xmm0, xmm0, xmm1
    ret

global AVXLog2F64
align 16
AVXLog2F64:
    vmovq           rax, xmm0
    mov             rcx, rax
    shr             rax, 52
    sub             eax, 1023
    vcvtsi2sd       xmm1, xmm1, eax
    mov             rax, 000FFFFFFFFFFFFFh
    and             rcx, rax
    mov             rax, 3FF0000000000000h
    or              rcx, rax
    vmovq           xmm0, rcx
    mov             rax, 3FF0000000000000h
    vmovq           xmm2, rax
    vsubsd          xmm0, xmm0, xmm2
    mov             rax, 3CAF67C5B70C0634h
    vmovq           xmm3, rax
    mov             rax, 3CD1A763E9D95D00h
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 0BD30D555AE4A500h
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FD2776C50EF9BFEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 0BFD71547652B82FEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FDEC709DC3A03FDh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    mov             rax, 3FF71547652B82FEh
    vmovq           xmm4, rax
    vfmadd213sd     xmm3, xmm0, xmm4
    vmulsd          xmm3, xmm3, xmm0
    vaddsd          xmm0, xmm1, xmm3
    ret

global AVXSinF64
align 16
AVXSinF64:
    vmovq           rax, xmm0
    mov             rcx, rax
    mov             r11, 8000000000000000h
    and             rcx, r11
    mov             [rsp - 8], rcx
    and             rax, 7FFFFFFFh
    vmovq           xmm0, rax
    mov             rax, 3FE45F306DC9C883h
    vmovq           xmm1, rax
    vmulsd          xmm1, xmm1, xmm0
    vroundsd        xmm2, xmm1, xmm1, 0
    vcvttsd2si      edx, xmm2
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm3, rax
    vmulsd          xmm3, xmm3, xmm2
    vsubsd          xmm0, xmm0, xmm3
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 0BE5AE5E68A2B9CEBh
    vmovq           xmm4, rax
    mov             rax, 3EC71DE3A556C734h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 0BF2A01A019C161D5h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 3F8111111110F8A6h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 0BFC5555555555549h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 3FF0000000000000h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    vmulsd          xmm4, xmm4, xmm0
    mov             rax, 3E21EE9EBDB4B1C4h
    vmovq           xmm6, rax
    mov             rax, 0BE927E4F809C52ADh
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 3EFA01A019CB1590h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 0BF56C16C16C15177h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 3FF0000000000000h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    and             edx, 3
    cmp             edx, 0
    je          @avxsin64_q0
    cmp             edx, 1
    je          @avxsin64_q1
    cmp             edx, 2
    je          @avxsin64_q2
    mov             rax, 800000000000000h
    vmovq           xmm0, rax
    vxorpd          xmm4, xmm6, xmm0
    jmp         @avxsin64_done

@avxsin64_q0:
    jmp         @avxsin64_done

@avxsin64_q1:
    vmovapd         xmm4, xmm6
    jmp         @avxsin64_done

@avxsin64_q2:
    mov             rax, 8000000000000000h
    vmovq           xmm0, rax
    vxorpd          xmm4, xmm4, xmm0
    
@avxsin64_done:
    mov             rcx, [rsp - 8]
    vmovq           xmm0, rcx
    vxorpd          xmm0, xmm0, xmm4
    ret

global AVXCosF64
align 16
AVXCosF64:
    vmovq           rax, xmm0
    mov             rcx, 7FFFFFFFFFFFFFFFh
    and             rax, rcx
    vmovq           xmm0, rax
    mov             rax, 3FE45F306DC9C883h
    vmovq           xmm1, rax
    vmulsd          xmm1, xmm1, xmm0
    vroundsd        xmm2, xmm1, xmm1, 0
    vcvttsd2si      edx, xmm2
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm3, rax
    vmulsd          xmm3, xmm3, xmm2
    vsubsd          xmm0, xmm0, xmm3
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 0BE5AE5E68A2B9CEBh
    vmovq           xmm4, rax
    mov             rax, 3EC71DE3A556C734h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 0BF2A01A019C161D5h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 3F8111111110F8A6h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 0BFC5555555555549h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    mov             rax, 3FF0000000000000h
    vmovq           xmm5, rax
    vfmadd213sd     xmm4, xmm1, xmm5
    vmulsd          xmm4, xmm4, xmm0
    mov             rax, 3E21EE9EBDB4B1C4h
    vmovq           xmm6, rax
    mov             rax, 0BE927E4F809C52ADh
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 3EFA01A019CB1590h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 0BF56C16C16C15177h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    mov             rax, 3FF0000000000000h
    vmovq           xmm7, rax
    vfmadd213sd     xmm6, xmm1, xmm7
    and             edx, 3
    cmp             edx, 0
    je          @avxcos64_q0
    cmp             edx, 1
    je          @avxcos64_q1
    cmp             edx, 2
    je          @avxcos64_q2
    vmovapd         xmm0, xmm4
    ret

@avxcos64_q0:
    vmovapd         xmm0, xmm6
    ret

@avxcos64_q1:
    mov             rax, 8000000000000000h
    vmovq           xmm0, rax
    vxorpd          xmm0, xmm0, xmm4
    ret

@avxcos64_q2:
    mov             rax, 8000000000000000h
    vmovq           xmm0, rax
    vxorpd          xmm0, xmm0, xmm6
    ret

global AVXTanF64
align 16
AVXTanF64:
    sub             rsp, 32
    vmovsd          qword [rsp], xmm0
    call        AVXSinF64
    vmovsd          qword [rsp - 16], xmm0
    vmovsd          xmm0, qword [rsp]
    call        AVXCosF64
    vmovsd          xmm1, qword [rsp - 16]
    vdivsd          xmm0, xmm1, xmm0
    add             rsp, 32
    ret

global AVXArcSinF64
align 16
AVXArcSinF64:
    vmovq           rax, xmm0
    mov             rcx, rax
    mov             r11, 8000000000000000h
    and             rcx, r11
    mov             r11, 7FFFFFFFFFFFFFFFh
    and             rax, r11
    vmovq           xmm0, rax
    mov             rax, 3FE0000000000000h
    vmovq           xmm1, rax
    vcomisd         xmm0, xmm1
    ja              @avxasin64_large
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 3F9F1C72E13AD8BEh
    vmovq           xmm2, rax
    mov             rax, 3FA6DB6DB3B44600h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FB3333333331C55h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FC5555555555555h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FF0000000000000h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    vmulsd          xmm0, xmm0, xmm2
    jmp             @avxasin64_done
    
@avxasin64_large:
    sub             rsp, 16
    mov             [rsp - 8], rcx
    mov             rax, 3FF0000000000000h
    vmovq           xmm1, rax
    vsubsd          xmm1, xmm1, xmm0
    mov             rax, 3FE0000000000000h
    vmovq           xmm2, rax
    vmulsd          xmm1, xmm1, xmm2
    vsqrtsd         xmm0, xmm0, xmm1
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 3F9F1C72E13AD8BEh
    vmovq           xmm2, rax
    mov             rax, 3FA6DB6DB3B44600h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FB3333333331C55h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FC5555555555555h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    mov             rax, 3FF0000000000000h
    vmovq           xmm3, rax
    vfmadd213sd     xmm2, xmm1, xmm3
    vmulsd          xmm0, xmm0, xmm2
    vaddsd          xmm0, xmm0, xmm0
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm1, rax
    vsubsd          xmm0, xmm1, xmm0
    mov             rcx, [rsp - 8]
    add             rsp, 16
    
@avxasin64_done:
    vmovq           xmm1, rcx
    vxorpd          xmm0, xmm0, xmm1
    ret

global AVXArcCosF64
align 16
AVXArcCosF64:
    call        AVXArcSinF64
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm1, rax
    vsubsd          xmm0, xmm1, xmm0
    ret

global AVXArcTanF64
align 16
AVXArcTanF64:
    vmovq           rax, xmm0
    mov             rcx, rax
    mov             r11, 8000000000000000h
    and             rcx, r11
    mov             r11, 7FFFFFFFFFFFFFFFh
    and             rax, r11
    vmovq           xmm0, rax
    mov             rax, 3FF0000000000000h
    vmovq           xmm1, rax
    vcomisd         xmm0, xmm1
    ja          @avxatan64_large
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 3FB745D171BF675Dh
    vmovq           xmm2, rax
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FBC71C6F5B4C600h
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm3, xmm3, xmm1
    mov             rax, 3FC2492492492492h
    vmovq           xmm2, rax
    vsubsd          xmm2, xmm2, xmm3
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FC999999999999Ah
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm3, xmm3, xmm1
    mov             rax, 3FD5555555555555h
    vmovq           xmm2, rax
    vsubsd          xmm2, xmm2, xmm3
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FF0000000000000h
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm0, xmm0, xmm3
    jmp         @avxatan64_done
    
@avxatan64_large:
    mov             rax, 3FF0000000000000h
    vmovq           xmm1, rax
    vdivsd          xmm0, xmm1, xmm0
    vmulsd          xmm1, xmm0, xmm0
    mov             rax, 3FB745D171BF675Dh
    vmovq           xmm2, rax
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FBC71C6F5B4C600h
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm3, xmm3, xmm1
    mov             rax, 3FC2492492492492h
    vmovq           xmm2, rax
    vsubsd          xmm2, xmm2, xmm3
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FC999999999999Ah
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm3, xmm3, xmm1
    mov             rax, 3FD5555555555555h
    vmovq           xmm2, rax
    vsubsd          xmm2, xmm2, xmm3
    vmulsd          xmm2, xmm2, xmm1
    mov             rax, 3FF0000000000000h
    vmovq           xmm3, rax
    vsubsd          xmm3, xmm3, xmm2
    vmulsd          xmm0, xmm0, xmm3
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm1, rax
    vsubsd          xmm0, xmm1, xmm0
    
@avxatan64_done:
    vmovq           xmm1, rcx
    vxorpd          xmm0, xmm0, xmm1
    ret

global AVXArcTan2F64
align 16
AVXArcTan2F64:
    sub             rsp, 32
    vmovq           rax, xmm0
    mov             rcx, rax
    mov             r11, 8000000000000000h
    and             rcx, r11
    mov             [rsp], rcx
    vmovq           rdx, xmm1
    mov             r8, rdx
    and             r8, r11
    mov             [rsp - 8], r8
    mov             r11, 7FFFFFFFFFFFFFFFh
    and             rax, r11
    and             rdx, r11
    vmovq           xmm0, rax
    vmovq           xmm1, rdx
    test            rdx, rdx
    jz          @avxatan264_xzero
    vdivsd          xmm0, xmm0, xmm1
    vmovsd          qword [rsp - 16], xmm0
    call        AVXArcTanF64
    mov             rcx, [rsp]
    mov             r8, [rsp - 8]
    test            r8, r8
    jz          @avxatan264_xpos
    mov             rax, 400921FB54442D18h
    vmovq           xmm1, rax
    test            rcx, rcx
    jz          @avxatan264_xneg_ypos
    vsubsd          xmm0, xmm0, xmm1
    jmp         @avxatan264_done

@avxatan264_xneg_ypos:
    mov             rax, 8000000000000000h
    vmovq           xmm2, rax
    vxorpd          xmm0, xmm0, xmm2
    vaddsd          xmm0, xmm0, xmm1
    jmp         @avxatan264_done
    
@avxatan264_xpos:
    vmovq           xmm1, rcx
    vxorpd          xmm0, xmm0, xmm1
    jmp         @avxatan264_done
    
@avxatan264_xzero:
    mov             rax, 3FF921FB54442D18h
    vmovq           xmm0, rax
    mov             rcx, [rsp]
    vmovq           xmm1, rcx
    vxorpd          xmm0, xmm0, xmm1
    
@avxatan264_done:
    add             rsp, 32
    ret

;; vector-types________________________________________________________________
global AVXExpF256
align 16
AVXExpF256:
    mov             eax, 3FB8AA3Bh
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vmulps          ymm0, ymm0, ymm1
    vroundps        ymm1, ymm0, 0
    vsubps          ymm0, ymm0, ymm1
    vcvttps2dq      ymm2, ymm1
    mov             eax, 7Fh
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vpaddd          ymm2, ymm2, ymm3
    vpslld          ymm2, ymm2, 23
    mov             eax, 3AAF047Bh
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    mov             eax, 3C1D9539h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm1, ymm0, ymm3
    mov             eax, 3D635847h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm1, ymm0, ymm3
    mov             eax, 3E75FDF0h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm1, ymm0, ymm3
    mov             eax, 3F317218h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm1, ymm0, ymm3
    mov             eax, 3F800000h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm1, ymm0, ymm3
    vmulps          ymm0, ymm1, ymm2
    ret

global AVXLnF256
align 16
AVXLnF256:
    vpsrld          ymm1, ymm0, 23
    mov             eax, 7Fh
    vmovd           xmm2, eax
    vbroadcastss    ymm2, xmm2
    vpsubd          ymm1, ymm1, ymm2
    vcvtdq2ps       ymm1, ymm1
    mov             eax, 007FFFFFh
    vmovd           xmm2, eax
    vbroadcastss    ymm2, xmm2
    vpand           ymm0, ymm0, ymm2
    mov             eax, 3F800000h
    vmovd           xmm2, eax
    vbroadcastss    ymm2, xmm2
    vpor            ymm0, ymm0, ymm2
    vsubps          ymm0, ymm0, ymm2
    mov             eax, 3E93DD98h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    mov             eax, 0BE98AA3Bh
    vmovd           xmm4, eax
    vbroadcastss    ymm4, xmm4
    vfmadd213ps     ymm3, ymm0, ymm4
    mov             eax, 3EF6384Fh
    vmovd           xmm4, eax
    vbroadcastss    ymm4, xmm4
    vfmadd213ps     ymm3, ymm0, ymm4
    mov             eax, 0BF38AA3Bh
    vmovd           xmm4, eax
    vbroadcastss    ymm4, xmm4
    vfmadd213ps     ymm3, ymm0, ymm4
    mov             eax, 3FB8AA3Bh
    vmovd           xmm4, eax
    vbroadcastss    ymm4, xmm4
    vfmadd213ps     ymm3, ymm0, ymm4
    vmulps          ymm3, ymm3, ymm0
    vaddps          ymm1, ymm1, ymm3
    mov             eax, 3F317218h
    vmovd           xmm0, eax
    vbroadcastss    ymm0, xmm0
    vmulps          ymm0, ymm0, ymm1
    ret

global AVXSinF256
align 16
AVXSinF256:
    mov             eax, 80000000h
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vandps          ymm7, ymm0, ymm1
    vandnps         ymm0, ymm1, ymm0
    mov             eax, 3F22F983h
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vmulps          ymm1, ymm1, ymm0
    vroundps        ymm2, ymm1, 0
    vcvttps2dq      ymm6, ymm2
    mov             eax, 3FC90FDBh
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vmulps          ymm3, ymm3, ymm2
    vsubps          ymm0, ymm0, ymm3
    vmulps          ymm1, ymm0, ymm0
    mov             eax, 0B9500D01h
    vmovd           xmm2, eax
    vbroadcastss    ymm2, xmm2
    mov             eax, 3C088889h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm2, ymm1, ymm3
    mov             eax, 0BE2AAAABh
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm2, ymm1, ymm3
    mov             eax, 3F800000h
    vmovd           xmm3, eax
    vbroadcastss    ymm3, xmm3
    vfmadd213ps     ymm2, ymm1, ymm3
    vmulps          ymm2, ymm2, ymm0
    mov             eax, 0BAB60B61h
    vmovd           xmm4, eax
    vbroadcastss    ymm4, xmm4
    mov             eax, 3D2AAAABh
    vmovd           xmm5, eax
    vbroadcastss    ymm5, xmm5
    vfmadd213ps     ymm4, ymm1, ymm5
    mov             eax, 0BF000000h
    vmovd           xmm5, eax
    vbroadcastss    ymm5, xmm5
    vfmadd213ps     ymm4, ymm1, ymm5
    mov             eax, 3F800000h
    vmovd           xmm5, eax
    vbroadcastss    ymm5, xmm5
    vfmadd213ps     ymm4, ymm1, ymm5
    mov             eax, 1
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vpand           ymm3, ymm6, ymm1
    vpcmpeqd        ymm3, ymm3, ymm1
    mov             eax, 2
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vpand           ymm5, ymm6, ymm1
    vpcmpeqd        ymm5, ymm5, ymm1
    vblendvps       ymm0, ymm2, ymm4, ymm3
    mov             eax, 80000000h
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vandps          ymm5, ymm5, ymm1
    vxorps          ymm0, ymm0, ymm5
    vxorps          ymm0, ymm0, ymm7
    ret

global AVXCosF256
align 16
AVXCosF256:
    mov             eax, 3FC90FDBh
    vmovd           xmm1, eax
    vbroadcastss    ymm1, xmm1
    vaddps          ymm0, ymm0, ymm1
    jmp         AVXSinF256
