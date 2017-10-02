
; Calling conversions. Taken from UNIX x86-64 ABI.
; RDI, RSI, RDX, RCX, R8, and R9
; RAX, R10 and R11 are available for use by the function without saving.
; Return value is stored in RAX and RDX
; Floating-point arguments are passed in XMM0 to XMM7
; If the callee wishes to use registers RBP, RBX, and R12–R15, it must restore
; - their original values before returning control to the caller
; If the callee is a variadic function, then the number of floating point
; - arguments passed to the function in vector registers must be provided by
; - the caller in the RAX register.
; %rax temporary register; with variable arguments
; passes information about the number of vector

;------------------------------------------------------------------------------------
; Regi      ;                                                   ; Preserved across  ;
; ster      ;   Usage                                           ; function calls    ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ; temporary register; with variable arguments       ;                   ;
;   rax     ; passes information about the number of vector     ;       No          ;
;           ; registers used; 1st return register               ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   rbx     ; callee-saved register                             ;       Yes         ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   rcx     ; used to pass 4th integer argument to functions    ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ; used to pass 3rd argument to functions; 2nd return;       No          ;
;   rdx     ; register                                          ;                   ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   rsp     ; stack pointer                                     ;       Yes         ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ; callee-saved register; optionally used as frame   ;       Yes         ;
;   rbp     ; pointer                                           ;                   ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   rsi     ; used to pass 2nd argument to functions            ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   rdi     ; used to pass 1st argument to functions            ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   r8      ; used to pass 5th argument to functions            ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   r9      ; used to pass 6th argument to functions            ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ; temporary register, used for passing a function’s ;       No          ;
;   r10     ; static chain pointer                              ;                   ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   r11     ; temporary register                                ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;  r12-r14  ; callee-saved registers                            ;       Yes         ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ; callee-saved register; optionally used as GOT     ;       Yes         ;
;   r15     ; base pointer                                      ;                   ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
; xmm0–xmm1 ; used to pass and return floating point arguments  ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
; xmm2–xmm7 ; used to pass floating point arguments             ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;xmm8–xmm15 ; temporary registers                               ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
; xmm0–xmm7 ; temporary registers                               ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
; xmm0–xmm7 ; temporary registers                               ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
; mmx0–mmx7 ; temporary registers                               ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;  st0,st1  ; temporary registers; used to return long          ;       No          ;
;           ; double arguments                                  ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;  st2–st7  ; temporary registers                               ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   fs      ; Reserved for system                               ;       No          ;
;           ; (as thread specific data register)                ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;   mxcsr   ; SSE2 control and status word                      ;       Partial     ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;  x87 SW   ; x87 status word                                   ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;  x87 CW   ; x87 control word                                  ;       No          ;
;           ;                                                   ;                   ;
;           ;                                                   ;                   ;
;------------------------------------------------------------------------------------

; Enable relative RIP adressing for being able to make shared object on x86-64 Linux
DEFAULT REL

;void surface_set_pixels(surface* surf, uint32_t x, uint32_t y, uint32_t n, void* pix_data);
extern surface_set_pixels

section .rodata
    const_one_ss:       dd 1.0
    center_ss:          dd -0.5
    meta_name:          db 'Mandelbrot ASM Kernel [AVX2 FMA]',0
    meta_name_size      equ $-meta_name
    meta_ver_maj:       db '1',0
    meta_ver_maj_size   equ $-meta_ver_maj
    meta_ver_min:       db '0',0
    meta_ver_min_size   equ $-meta_ver_min
align 32
    v_iter_ps:          dd 0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0
    v_one_ps:           times 8 dd 1.0
    v_bound2_ps:        times 8 dd 4.0

section .data
    width:              dd 0 ;1024
    height:             dd 0 ;1024
    ;wxh:                dq 1024*1024
    surface_ptr:        dq 0
    output_sz:          dd 0 ;1024*1024*4
    output_stride:      dd 0 ;1024*4

    scale_ss:           dd 0 ;0.00188964
    shift_x_ss:         dd 0 ;-1.347385054652062
    shift_y_ss:         dd 0 ;0.063483549665202

    bailout_si:         dd 256

    width_r_ss:         dd 0 ;0.0009765625 ; 1/1024
    height_r_ss:        dd 0 ;0.0009765625 ; 1/1024
    aspect_ss:          dd 0 ;1.0
align 32
    v_transpose_x_ps:   times 8 dd 0
    v_transpose_y_ps:   times 8 dd 0
    v_offset_y_ps:      times 8 dd 0
    v_offset_x_ps:      times 8 dd 0





;--------------------------------------------------
; Macro
;--------------------------------------------------
%define x_mm(mm) x %+ mm
%define y_mm(mm) y %+ mm

%macro  self_broadcast256_ss 1
        vshufps x_mm(%1),x_mm(%1),0                   ; replicate in xmm register
        vinsertf128 y_mm(%1), y_mm(%1), x_mm(%1), 1   ; copy low part to high part of ymm register
%endmacro

section .text
    global sample_rdtsc:function
    global sample_rdtscp:function

    global mdb_kernel_metadata_query:function

    global mdb_kernel_init:function
    global mdb_kernel_shutdown:function

    global mdb_kernel_cpu_features:function

    global mdb_kernel_process_block:function

    ; rdi = width
    ; rsi = height
    global mdb_kernel_set_size:function

    ; xmm0 - scale
    global mdb_kernel_set_scale:function

    ; xmm0 - shift_x
    ; xmm1 - shift_y
    global mdb_kernel_set_shift:function

    ; edi - bailout
    global mdb_kernel_set_bailout:function

    ; rdi - pointer to f32 buffer
    global mdb_kernel_set_surface:function

    ;void(void)
    global mdb_kernel_submit_changes:function

; old metadata query version
%if 0
; edi = int flags
; rsi = char* buff
; edx = int buffsize
mdb_kernel_metadata_query:
%push mdb_kernel_metadata_query
%define FLAG_NAME    1b
%define FLAG_VER_MAJ 10b
%define FLAG_VER_MIN 100b

%define flags        edi
%define buff         rsi
%define buffsize     edx
%define idx          r10
%define buff_idx     r9d
%define buff_idx_q   r9
%define cur_field    rcx
%define cur_field_sz r11
%define tmp0b        r8b

    xor buff_idx,buff_idx
.test_query_flags:
    mov eax,flags
    and  eax,FLAG_NAME
    test eax,eax
    jnz .query_name

    mov eax,flags
    and  eax,FLAG_VER_MAJ
    test eax,eax
    jnz .query_ver_maj

    mov eax,flags
    and  eax,FLAG_VER_MIN
    test eax,eax
    jnz .query_ver_min


    jmp .exit

.query_name:
    mov cur_field,meta_name
    mov cur_field_sz,meta_name_size
    xor flags,FLAG_NAME
    jmp .copy_to_buff

.query_ver_maj:
    mov cur_field,meta_ver_maj
    mov cur_field_sz,meta_ver_maj_size
    xor flags,FLAG_VER_MAJ
    jmp .copy_to_buff

.query_ver_min:
    mov cur_field,meta_ver_min
    mov cur_field_sz,meta_ver_min_size
    xor flags,FLAG_VER_MIN
    jmp .copy_to_buff

.copy_to_buff:
    xor idx,idx
.copy_loop:
    cmp buff_idx,buffsize
    jge .exit

    cmp idx,cur_field_sz
    jge .test_query_flags

    mov tmp0b,[cur_field + idx]
    mov [buff + buff_idx_q],tmp0b
    inc idx
    inc buff_idx
    jmp .copy_loop

.exit:
    xor eax,eax
    mov [buff + buff_idx_q],eax
    ret
%pop
%endif

; edi = int flags
; rsi = char* buff
; edx = int buffsize
mdb_kernel_metadata_query:
%push mdb_kernel_metadata_query
%define FLAG_NAME    1
%define FLAG_VER_MAJ 2
%define FLAG_VER_MIN 3

%define flags        edi
%define buff         rsi
%define buffsize     edx
%define idx          r10
%define buff_idx     r9d
%define buff_idx_q   r9
%define cur_field    rcx
%define cur_field_sz r11
%define tmp0b        r8b

    xor buff_idx,buff_idx
.test_query_flags:
    mov eax,flags

    cmp eax,FLAG_NAME
    je .query_name

    cmp  eax,FLAG_VER_MAJ
    je .query_ver_maj

    cmp  eax,FLAG_VER_MIN
    je .query_ver_min


    jmp .exit

.query_name:
    mov cur_field,meta_name
    mov cur_field_sz,meta_name_size
    xor flags,FLAG_NAME
    jmp .copy_to_buff

.query_ver_maj:
    mov cur_field,meta_ver_maj
    mov cur_field_sz,meta_ver_maj_size
    xor flags,FLAG_VER_MAJ
    jmp .copy_to_buff

.query_ver_min:
    mov cur_field,meta_ver_min
    mov cur_field_sz,meta_ver_min_size
    xor flags,FLAG_VER_MIN
    jmp .copy_to_buff

.copy_to_buff:
    xor idx,idx
.copy_loop:
    cmp buff_idx,buffsize
    jge .exit

    cmp idx,cur_field_sz
    jge .exit

    mov tmp0b,[cur_field + idx]
    mov [buff + buff_idx_q],tmp0b
    inc idx
    inc buff_idx
    jmp .copy_loop

.exit:
    xor eax,eax
    mov [buff + buff_idx_q],eax
    ret
%pop

mdb_kernel_cpu_features:
; mdb/tools/cpu_features.h
; CPU_FEATURE_AVX2    = 1<<8,
; CPU_FEATURE_FMA     = 1<<9
    mov eax,1<<8 | 1<<9
    ret

mdb_kernel_init:
    ret

mdb_kernel_shutdown:
    ret

mdb_kernel_submit_changes:
    ;(height_r * cy + center) * scale + shift_y
    ;cy * height_r * scale + center * scale + shift_y
    ;a = height_r * scale
    ;b = center * scale + shift_y
    ;cy * a + b

    ;--------------------------------------
    ;vmovaps ymm0,[v_height_r_ps]
    ;vmulps  ymm0,ymm0,[v_scale_ps]
    ;vmovaps [v_transpose_y_ps],ymm0

    ;vmovaps      ymm0,[v_center_ps]
    ;vmovaps      ymm1,[v_scale_ps]
    ;vfmadd213ps  ymm0,ymm1,[v_shift_y_ps]
    ;vmovaps [v_offset_y_ps],ymm0
    ;--------------------------------------

    vmovss xmm0,[height_r_ss]
    vmulss xmm0,xmm0,[scale_ss]

    self_broadcast256_ss mm0
    vmovaps [v_transpose_y_ps],ymm0

    vmovss xmm0,[center_ss]
    vmulss xmm0,xmm0,[scale_ss]
    vaddss xmm0,xmm0,[shift_y_ss]

    self_broadcast256_ss mm0
    vmovaps [v_offset_y_ps],ymm0

    ;--------------------------------------

    ; (width_r * cx + center) * aspect * scale + shift_x
    ; scale_x = aspect * scale
    ; (width_r * cx + center) * scale_x + shift_x
    ; cx * width_r * scale_x + center * scale_x + shift_x
    ; a = width_r * scale_x
    ; b = center * scale_x + shift_x
    ; cx * a + b

    ;--------------------------------------
    ; ymm0 = scale_x
    ;vmovaps ymm0,[v_aspect_ps]
    ;vmulps  ymm0,ymm0,[v_scale_ps]

    ;vmulps ymm1,ymm0,[v_width_r_ps]
    ;vmovaps [v_transpose_x_ps],ymm1

    ;vmovaps      ymm1,[v_center_ps]
    ;vfmadd213ps  ymm0,ymm1,[v_shift_x_ps]
    ;vmovaps [v_offset_x_ps],ymm0
    ;--------------------------------------

    ; xmm0 = scale_x
    vmovss xmm0,[scale_ss]
    vmulss xmm0,xmm0,[aspect_ss]

    vmulss xmm1,xmm0,[width_r_ss]

    self_broadcast256_ss mm1
    vmovaps [v_transpose_x_ps],ymm1

    vmulss xmm0,xmm0,[center_ss]
    vaddss xmm0,xmm0,[shift_x_ss]

    self_broadcast256_ss mm0
    vmovaps [v_offset_x_ps],ymm0
    ;--------------------------------------

    ret

; edi = width
; esi = height
mdb_kernel_set_size:
    mov [width], edi
    mov [height],esi

    ;mov rax,rdi
    ;imul rax,rsi
    ;mov [wxh],rax

    mov eax,edi
    imul eax,4          ; stride width*4 bytes
    mov [output_stride],eax

    vcvtsi2ss xmm0,edi ; width
    vcvtsi2ss xmm1,esi ; height

    vdivss xmm2,xmm0,xmm1
    vmovss [aspect_ss],xmm2

    vmovss xmm2,[const_one_ss]
    vdivss xmm0,xmm2,xmm0
    vdivss xmm1,xmm2,xmm1

    vmovss [width_r_ss], xmm0
    vmovss [height_r_ss],xmm1

    ret

; xmm0 - scale
mdb_kernel_set_scale:
    vmovss [scale_ss],xmm0
    ret

; xmm0 - shift_x
; xmm1 - shift_y
mdb_kernel_set_shift:
    vmovss [shift_x_ss],xmm0
    vmovss [shift_y_ss],xmm1
    ret

; edi - bailout
mdb_kernel_set_bailout:
    mov [bailout_si],edi
    ret

; rdi - pointer to surface structure
mdb_kernel_set_surface:
    mov [surface_ptr],rdi
    ret


; return tsc in rax
sample_rdtsc:
    push rbx

    cpuid
    rdtsc

    mov rbx,32
    shlx rdx,rdx,rbx
    or rax,rdx

    pop rbx
    ret

; return tsc in rax
sample_rdtscp:
    push rbx

    rdtscp

    mov rbx,32
    shlx rdx,rdx,rbx
    or rax,rdx
    push rax

    cpuid

    pop rax
    pop rbx
    ret

;int32 x0, int32 x1, int32 y0, int32 y1
; edi x0
; esi x1
; edx y0
; ecx y1
mdb_kernel_process_block:
%push mdb_kernel_process_block

%define _CMP_LT_OQ  0x11
%define _CMP_NEQ_OQ 0x0c

%define tmp0_b      r9b
%define tmp0_w      r9w
%define tmp0_d      r9d
%define tmp0        r9
%define tmp1        r10
%define tmp1_d      r10d
%define tmp2        r11
%define tmp2_d      r11d
%define bailout     r12d
%define i           r13d
%define x_init      r14d
%define x           edi
%define x_q         rdi
%define x1          esi
%define y           edx
%define y_q         rdx
%define y1          ecx
%define v_cy_l      xmm0
%define v_cy        ymm0
%define v_cx_l      xmm1
%define v_cx        ymm1
%define v_zy        ymm2
%define v_zx        ymm3
%define v_i         ymm4
%define v_zx2       ymm5
%define v_zy2_cx    ymm6
%define v_zxzy_cy   ymm7
%define v_zx_old    ymm8
%define v_iter      ymm9
%define v_offset_x  ymm10
%define v_bound2    ymm11
%define v_one       ymm12
%define v_transpose_x ymm13
%define v_tmp1      ymm14
%define v_tmp0      ymm15
%define v_tmp0_l    xmm15


    ; setup stack frame
    push rbp
    mov rbp,rsp

    ; save calle-save registers
    push rbx
    push r15
    push r14
    push r13
    push r12
    push r11

    ; Initialize data
    mov bailout,[bailout_si]

    vmovaps v_bound2,[v_bound2_ps]
    vmovaps v_one,[v_one_ps]
    vmovaps v_transpose_x,[v_transpose_x_ps]
    vmovaps v_offset_x,[v_offset_x_ps]
    vmovaps v_iter,[v_iter_ps]

    ; save initial value of x
    mov x_init,x
.loop_y:
    ; set x to initial
    mov x,x_init

    vcvtsi2ss v_cy_l,y
    vbroadcastss v_cy,v_cy_l

    vmovaps     v_tmp0,[v_transpose_y_ps]
    vfmadd213ps v_cy,v_tmp0,[v_offset_y_ps]

.loop_x:
    vcvtsi2ss v_cx_l,x
    vbroadcastss v_cx,v_cx_l
    vaddps v_cx,v_cx,v_iter


    vfmadd213ps v_cx,v_transpose_x,v_offset_x

    vmovaps v_zx,v_cx
    vmovaps v_zy,v_cy

    ; don't forget to reset i to zero
    xor i,i
    vxorps v_i,v_i,v_i
.loop_bailout:
%if 0
    ;zx2_cx = zy * zy - cx
    vmovaps     v_tmp0,v_zy
    vfmsub213ps v_tmp0,v_tmp0,v_cx

    ;zx1 = zx * zx - zy2_cx
    vmovaps     v_tmp1,v_zx
    vfmsub213ps v_tmp1,v_tmp1,v_tmp0

    ;zxzy_cy = zx * zy + cy
    vmovaps     v_tmp0,v_zx
    vfmadd213ps v_tmp0,v_zy,v_cy

    ;zy1 = zx * zy + zxzy_cy
    vfmadd213ps v_zy,v_zx,v_tmp0

    vmovaps     v_zx,v_tmp1

    ;mag2 = zx1 * zx1 + zy1 * zy1
    ;mag2 = fma(zx1,zx1, mul(zy1,zy1))
    ;mag2 = zx1
    vmulps      v_tmp0,v_zy,v_zy
    ;vmulps      v_tmp1,v_zx,v_zx
    ;vaddps      v_tmp0,v_tmp0,v_tmp1
    vfmadd213ps v_tmp1,v_tmp1,v_tmp0

    ; v_zx1 = mag2
    ; bound_mask = v_zx1
    vcmpps v_tmp1,v_tmp1,v_bound2,_CMP_LT_OQ

    vtestps v_tmp1,v_tmp1
    jz .loop_bailout_exit

    ; add_mask = v_zx1
    vandps v_tmp1,v_tmp1,v_one
    vaddps v_i,v_i,v_tmp1

    inc i
    cmp i,bailout
    jne  .loop_bailout

%else
    vmovaps      v_zx_old,v_zx

    ;zy2_cx = zy * zy - cx
    vmovaps     v_tmp0,v_zy
    vfmsub213ps v_tmp0,v_tmp0,v_cx

    ;zx1 = zx * zx - zy2_cx
    vfmsub213ps v_zx,v_zx,v_tmp0

    ;zxzy_cy = zx * zy + cy
    vmovaps     v_tmp0,v_zx_old
    vfmadd213ps v_tmp0,v_zy,v_cy

    ;zy1 = zx * zy + zxzy_cy
    vfmadd213ps v_zy,v_zx_old,v_tmp0

    vmovaps     v_tmp0,v_zx
    vmulps      v_tmp1,v_zy,v_zy
    vfmadd213ps v_tmp0,v_tmp0,v_tmp1

    vcmpps v_tmp0,v_tmp0,v_bound2,_CMP_LT_OQ

    ; add_mask = v_zx1
    vandps v_tmp0,v_tmp0,v_one
    vaddps v_i,v_i,v_tmp0


    vptest v_tmp0,v_tmp0
    jz .loop_bailout_exit

    inc  i
    cmp i,bailout
    jne  .loop_bailout
%endif



.loop_bailout_exit:

    ;vxorps v_tmp0,v_tmp0,v_tmp0

    ; bailout = v_tmp0
    vcvtsi2ss v_tmp0_l,bailout
    vbroadcastss v_tmp0,v_tmp0_l

    ; bailout_mask = v_tmp1
    ; zero if bailed out
    vcmpps v_tmp1,v_i,v_tmp0,_CMP_NEQ_OQ

    ; zero out bailed out elements
    vandps v_i,v_i,v_tmp1

    ; normalize values
    vdivps v_i,v_i,v_tmp0

    ;---------------------------------------
    ; Begin function call block
    ;---------------------------------------

    ; save general purpose register before call
    push rdi
    push rsi
    push rdx
    push rcx

    ; set up new stack frame
    ; to change stack align to 32
    push rbp
    mov rbp,rsp

    ; set stack alignment 32
    and rsp,-32

    ; allocate space on stack for vector register
    sub rsp,256*8

    ; save vector registers
    vmovaps [rsp], v_bound2
    vmovaps [rsp+256*1], v_one
    vmovaps [rsp+256*2], v_transpose_x
    vmovaps [rsp+256*3], v_offset_x
    vmovaps [rsp+256*4], v_iter
    vmovaps [rsp+256*5], v_cy
    vmovaps [rsp+256*6], v_cx
    vmovaps [rsp+256*7],v_i

    ; calling function signature
    ;void surface_set_pixels(surface* surf, uint32_t x, uint32_t y, uint32_t n, void* pix_data);

    ; set up input arguments

    ; set up 2nd arg first to not overwrite x in rdi
    mov rsi,x_q

    ; y - rdx so rdx=rdx pointless
    ;mov rdx,y_q

    mov rdi,[surface_ptr]
    mov rcx,8
    lea r8,[rsp+256*7]

    ; set up new stack frame to restore alignment before call
    ; allocate 32 bytes to save rbp without breaking alignment
    sub rsp,32
    mov [rsp],rbp
    mov rbp,rsp

    ; restore stack aligment to 16
    and rsp,-16

    ; call the function to set pixels
    call surface_set_pixels wrt ..plt

    ; restore stack with 32 aligment
    mov rsp,rbp
    mov rbp,[rsp]

    ; deallocate space for rbp
    add rsp,32

    ; restore vector registers
    vmovaps v_bound2,       [rsp]
    vmovaps v_one,          [rsp+256*1]
    vmovaps v_transpose_x,  [rsp+256*2]
    vmovaps v_offset_x,     [rsp+256*3]
    vmovaps v_iter,         [rsp+256*4]
    vmovaps v_cy,           [rsp+256*5]
    vmovaps v_cx,           [rsp+256*6]

    ; deallocate space of vector registers
    add rsp,256*8

    ; restore stack with 16 byte alignment
    mov rsp,rbp
    pop rbp

    ; restore general purpose registers
    pop rcx
    pop rdx
    pop rsi
    pop rdi

    ;---------------------------------------
    ; end of function call block
    ;---------------------------------------

    add x,8
    cmp x,x1
    jle .loop_x

    inc y
    cmp y,y1
    jle .loop_y

    ; restore calle-save registers
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbx

    ; restore stack
    mov rsp,rbp
    pop rbp

    xor rax,rax
    ret
%pop
