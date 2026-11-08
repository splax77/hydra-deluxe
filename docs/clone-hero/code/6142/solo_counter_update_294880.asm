00294880: push   rbx
00294882: push   rbp
00294883: push   rsi
00294884: push   rdi
00294885: sub    rsp, 0x38
00294889: cmp    byte ptr [rip + 0x3c78f6d], 0            ; [0x3f0d7fd] (bss)
00294890: mov    esi, r9d
00294893: movzx  ebp, r8b
00294897: mov    edi, edx
00294899: mov    rbx, rcx
0029489c: jne    0x1802948b1
0029489e: lea    rcx, [rip + 0x3a3d693]                   ; [0x3cd1f38] meta:SoloCounter_TypeInfo
002948a5: call   0x182f609b0
002948aa: mov    byte ptr [rip + 0x3c78f4c], 1            ; [0x3f0d7fd] (bss)
002948b1: movsd  xmm0, qword ptr [rsp + 0x80]
002948ba: movsd  qword ptr [rbx + 0x48], xmm0
002948bf: mov    byte ptr [rbx + 0x50], 0
002948c3: mov    rax, qword ptr [rip + 0x3a3d66e]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
002948ca: mov    rcx, qword ptr [rax + 0xb8]
002948d1: mov    rdx, qword ptr [rcx]
002948d4: cmp    edi, 8
002948d7: je     0x180294934
002948d9: cmp    edi, -0x12
002948dc: jge    0x18029491b
002948de: cmp    edi, -0x30
002948e1: jge    0x180294934
002948e3: cmp    edi, -0xb
002948e6: je     0x1802948ff
002948e8: test   rdx, rdx
002948eb: je     0x1802949f1
002948f1: cmp    dword ptr [rdx + 0x18], 0
002948f5: jbe    0x1802949f7
002948fb: xor    esi, esi
002948fd: jmp    0x180294947
002948ff: test   rdx, rdx
00294902: je     0x1802949f1
00294908: cmp    dword ptr [rdx + 0x18], -0x35
0029490c: jbe    0x1802949f7
00294912: mov    rdx, qword ptr [rdx - 0x188]
00294919: jmp    0x18029494b
0029491b: test   rdx, rdx
0029491e: je     0x1802949f1
00294924: cmp    dword ptr [rdx + 0x18], 1
00294928: jbe    0x1802949f7
0029492e: mov    rdx, qword ptr [rdx + 0x28]
00294932: jmp    0x18029494b
00294934: test   rdx, rdx
00294937: je     0x1802949f1
0029493d: cmp    dword ptr [rdx + 0x18], 0
00294941: jbe    0x1802949f7
00294947: mov    rdx, qword ptr [rdx + 0x20]
0029494b: mov    rax, rdx
0029494e: lea    rcx, [rbx + 0x78]
00294952: mov    qword ptr [rbx + 0x78], rdx
00294956: call   0x182f5fc00
0029495b: test   bpl, bpl
0029495e: je     0x18029498d
00294960: mov    rax, qword ptr [rip + 0x3a3d5d1]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
00294967: mov    rcx, qword ptr [rax + 0xb8]
0029496e: mov    rdx, qword ptr [rcx]
00294971: test   rdx, rdx
00294974: je     0x1802949f1
00294976: cmp    dword ptr [rdx + 0x18], 7
0029497a: jbe    0x1802949f7
0029497c: mov    rdx, qword ptr [rdx + 0x58]
00294980: lea    rcx, [rbx + 0x78]
00294984: mov    qword ptr [rbx + 0x78], rdx
00294988: call   0x182f5fc00
0029498d: mov    dword ptr [rbx + 0x80], esi
00294993: cmp    byte ptr [rip + 0x3c77fe5], 0            ; [0x3f0c97f] (bss)
0029499a: mov    rbx, qword ptr [rbx + 0x30]
0029499e: jne    0x1802949b3
002949a0: lea    rcx, [rip + 0x3a5ca31]                   ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
002949a7: call   0x182f609b0
002949ac: mov    byte ptr [rip + 0x3c77fcc], 1            ; [0x3f0c97f] (bss)
002949b3: test   rbx, rbx
002949b6: je     0x1802949f1
002949b8: mov    rax, qword ptr [rip + 0x3a5ca19]         ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
002949bf: lea    rdx, [rsp + 0x20]
002949c4: xor    r8d, r8d
002949c7: mov    rcx, rbx
002949ca: mov    rax, qword ptr [rax + 0xb8]
002949d1: movsd  xmm0, qword ptr [rax + 0xc]
002949d6: mov    eax, dword ptr [rax + 0x14]
002949d9: movsd  qword ptr [rsp + 0x20], xmm0
002949df: mov    dword ptr [rsp + 0x28], eax
002949e3: call   0x18288a650                              ; UnityEngine.Transform$$set_localScale
002949e8: add    rsp, 0x38
002949ec: pop    rdi
002949ed: pop    rsi
002949ee: pop    rbp
002949ef: pop    rbx
002949f0: ret    
002949f1: call   0x182f60c50
002949f6: int3   
002949f7: call   0x182f60c40
002949fc: int3   
002949fd: int3   
