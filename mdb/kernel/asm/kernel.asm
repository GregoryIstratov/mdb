
; calling conversions
; RDI, RSI, RDX, RCX, R8, and R9
; RAX, R10 and R11 are available for use by the function without saving.
; Return value is stored in RAX and RDX
; Floating-point arguments are passed in XMM0 to XMM7
;If the callee wishes to use registers RBP, RBX, and R12–R15, it must restore
;- their original values before returning control to the caller
;If the callee is a variadic function, then the number of floating point
;- arguments passed to the function in vector registers must be provided by
;- the caller in the RAX register.
;%rax temporary register; with variable arguments
;passes information about the number of vector

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

extern printf
extern malloc
extern calloc
extern free

extern print8_ps
extern save_surface
extern test_timers
extern set_def_loc
extern sample_timer
extern print_clocks

section .rodata
    const_one_ss:       dd 1.0
    center_ss:          dd -0.5
align 32
    v_iter_ps:          dd 0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0
    v_one_ps:           times 8 dd 1.0
    v_bound2_ps:        times 8 dd 4.0

section .data
    fmt_cycles:         db "Total cycles: %'lu",10,0
    fmt_pps:            db "Cycles/Pixel: %f",10,0
    width:              dq 0 ;1024
    height:             dq 0 ;1024
    ;wxh:                dq 1024*1024
    output_ptr:         dq 0
    output_sz:          dq 0 ;1024*1024*4
    output_stride:      dq 0 ;1024*4

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
    global mdbt_kernel:function
    global sample_rdtsc:function
    global sample_rdtscp:function

    ; rdi = width
    ; rsi = height
    global mdbt_set_size:function

    ; xmm0 - scale
    global mdbt_set_scale:function

    ; xmm0 - shift_x
    ; xmm1 - shift_y
    global mdbt_set_shift:function

    ; edi - bailout
    global mdbt_set_bailout:function

    ; rdi - pointer to f32 buffer
    global mdbt_set_surface:function

    ;void(void)
    global mdbt_compute_transpose_offset:function

mdbt_compute_transpose_offset:
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

; rdi = width
; rsi = height
mdbt_set_size:
    mov [width], rdi
    mov [height],rsi

    ;mov rax,rdi
    ;imul rax,rsi
    ;mov [wxh],rax

    mov rax,rdi
    imul rax,4          ; stride width*4 bytes
    mov [output_stride],rax

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
mdbt_set_scale:
    vmovss [scale_ss],xmm0
    ret

; xmm0 - shift_x
; xmm1 - shift_y
mdbt_set_shift:
    vmovss [shift_x_ss],xmm0
    vmovss [shift_y_ss],xmm1
    ret

; edi - bailout
mdbt_set_bailout:
    mov [bailout_si],edi
    ret

; rdi - pointer to f32 buffer
mdbt_set_surface:
    mov [output_ptr],rdi
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


;long x0, long x1, long y0, long y1
; rdi x0
; rsi x1
; rdx y0
; rcx y1
mdbt_kernel:
%push mdbt_kernel
%stacksize flat64
%assign %$localsize 0

%define _CMP_LT_OQ  0x11
%define _CMP_NEQ_OQ 0x0c

%define tmp0_b      r9b
%define tmp0_w      r9w
%define tmp0_d      r9d
%define tmp0        r9
%define tmp1        r10
%define bailout     r13d
%define i           r14d
%define x_init      r15d
%define output      r8
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

    enter %$localsize, 0
    push rbx
    push r15
    push r14
    push r13
    push r12

    ; Initialize data
    mov output,[output_ptr]
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
    ;vcvtsi2ss v_cy_l,y                          ; convert to single float and store to the low part of xmm
    ;vshufps v_cy_l,v_cy_l,0                     ; replicate in xmm register
    ;vinsertf128 v_cy, v_cy, v_cy_l, 1           ; copy low part to high part of ymm register

    vcvtsi2ss v_cy_l,y
    vbroadcastss v_cy,v_cy_l

    vmovaps     v_tmp0,[v_transpose_y_ps]
    vfmadd213ps v_cy,v_tmp0,[v_offset_y_ps]

.loop_x:
    vcvtsi2ss v_cx_l,x                          ; convert to single float and store to the low part of xmm
    ;vshufps v_cx_l,v_cx_l,0                     ; replicate in xmm register
    ;vinsertf128 v_cx, v_cx, v_cx_l, 1           ; copy low part to high part of ymm register
    vbroadcastss v_cx,v_cx_l
    vaddps v_cx,v_cx,v_iter                     ; increment


    vfmadd213ps v_cx,v_transpose_x,v_offset_x

    vmovaps v_zx,v_cx
    vmovaps v_zy,v_cy

    xor i,i                                     ; i = 0
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

    mov tmp0,y_q           ; y
    mov tmp1,x_q           ; x
    mov rax,[output_stride]

    imul rax,tmp0        ; stride*y

    imul tmp1,4          ; x * sizeof(float)
    add rax,tmp1         ; rax = y*stride + x*sizeof(float)

    ; write to buffer
    lea tmp0,[output + rax] ; tmp0 = output[y*4*stride+x*4]
    vmovups [tmp0],v_i

    add x,8
    cmp x,x1
    jle .loop_x

    inc y
    cmp y,y1
    jle .loop_y


    pop r12
    pop r13
    pop r14
    pop r15
    pop rbx

    ;mov rsp,rbp
    ;pop rbp                 ; restore stack

    xor rax,rax
    leave
    ret
%pop

global run_kernel:function

%if 0
run_kernel:
%push run_kernel
%stacksize flat64
%assign %$localsize 0
%local tsc_begin:qword,tsc_end:qword,i:qword,j:qword

    enter %$localsize,0

    call mdbt_compute_transpose_offset

    xor rax,rax
    mov [i],rax

    mov rdi,1
    mov rsi,[output_sz]
    call calloc
    mov [output_ptr],rax

    call sample_rdtsc
    call sample_rdtscp
    call sample_rdtsc
    call sample_rdtscp
    call sample_rdtsc
    call sample_rdtscp

    call sample_timer

    call sample_rdtsc
    mov [tsc_begin],rax

.loop_bench:
    mov rdi,0
    mov rsi,1023
    mov rdx,0
    mov rcx,1023
    call mdbt_kernel

    mov rax,[i]
    inc rax
    mov [i],rax
    cmp rax,100
    ;jne .loop_bench

    call sample_rdtscp

    mov rdi,[tsc_begin]
    mov rsi,rax
    call print_clocks

    call sample_timer

    mov rdi,[output_ptr]
    mov rsi,[output_sz]
    call save_surface

    leave

    xor rax,rax
    ret
%pop

%endif