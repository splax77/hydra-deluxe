002962c0: mov    qword ptr [rsp + 8], rbx
002962c5: push   rdi
002962c6: sub    rsp, 0x20
002962ca: cmp    byte ptr [rip + 0x3c77530], 0            ; [0x3f0d801] (bss)
002962d1: mov    rdi, rcx
002962d4: jne    0x1802962e9
002962d6: lea    rcx, [rip + 0x3a5dd3b]                   ; [0x3cf4018] meta:UnityEngine.WaitForSeconds_TypeInfo
002962dd: call   0x182f609b0
002962e2: mov    byte ptr [rip + 0x3c77518], 1            ; [0x3f0d801] (bss)
002962e9: mov    byte ptr [rdi + 0x50], 1
002962ed: mov    rcx, qword ptr [rip + 0x3a5dd24]         ; [0x3cf4018] meta:UnityEngine.WaitForSeconds_TypeInfo
002962f4: call   0x182f60c00
002962f9: movss  xmm1, dword ptr [rip + 0x2dcf66f]        ; [0x3065970] flt=0.03200000151991844 i=1023611503
00296301: xor    r8d, r8d
00296304: mov    rcx, rax
00296307: mov    rbx, rax
0029630a: call   0x18288f770                              ; UnityEngine.WaitForSeconds$$.ctor
0029630f: lea    rcx, [rdi + 0x68]
00296313: mov    qword ptr [rdi + 0x68], rbx
00296317: mov    rdx, rbx
0029631a: call   0x182f5fc00
0029631f: mov    rcx, qword ptr [rip + 0x3a5dcf2]         ; [0x3cf4018] meta:UnityEngine.WaitForSeconds_TypeInfo
00296326: call   0x182f60c00
0029632b: movss  xmm1, dword ptr [rip + 0x2dcf4fd]        ; [0x3065830] flt=1.5 i=1069547520
00296333: xor    r8d, r8d
00296336: mov    rcx, rax
00296339: mov    rbx, rax
0029633c: call   0x18288f770                              ; UnityEngine.WaitForSeconds$$.ctor
00296341: lea    rcx, [rdi + 0x70]
00296345: mov    qword ptr [rdi + 0x70], rbx
00296349: mov    rdx, rbx
0029634c: call   0x182f5fc00
00296351: xor    edx, edx
00296353: mov    rcx, rdi
00296356: mov    rbx, qword ptr [rsp + 0x30]
0029635b: add    rsp, 0x20
0029635f: pop    rdi
00296360: jmp    0x182877c30                              ; UnityEngine.Transform$$.ctor
00296365: int3   
