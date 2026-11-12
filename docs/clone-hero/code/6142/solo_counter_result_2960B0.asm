002960b0: push   rbx
002960b2: push   rbp
002960b3: push   rsi
002960b4: push   rdi
002960b5: sub    rsp, 0x38
002960b9: cmp    byte ptr [rip + 0x3c77748], 0            ; [0x3f0d808] (bss)
002960c0: mov    esi, r9d
002960c3: movzx  ebp, r8b
002960c7: mov    ebx, edx
002960c9: mov    rdi, rcx
002960cc: jne    0x1802960e1
002960ce: lea    rcx, [rip + 0x3a3be63]                   ; [0x3cd1f38] meta:SoloCounter_TypeInfo
002960d5: call   0x182f609b0
002960da: mov    byte ptr [rip + 0x3c77727], 1            ; [0x3f0d808] (bss)
002960e1: movsd  xmm0, qword ptr [rsp + 0x80]
002960ea: movsd  qword ptr [rdi + 0x48], xmm0
002960ef: mov    byte ptr [rdi + 0x50], 1
002960f3: mov    rax, qword ptr [rip + 0x3a3be3e]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
002960fa: mov    rcx, qword ptr [rax + 0xb8]
00296101: mov    rdx, qword ptr [rcx]
00296104: cmp    ebx, 0x64
00296107: je     0x1802961ea
0029610d: cmp    ebx, 0x5f
00296110: jge    0x1802961d1
00296116: cmp    ebx, 0x5a
00296119: jge    0x1802961b8
0029611f: cmp    ebx, 0x50
00296122: jge    0x18029619f
00296124: cmp    ebx, 0x46
00296127: jge    0x180296186
00296129: cmp    ebx, 0x45
0029612c: je     0x18029616d
0029612e: cmp    ebx, 0x3c
00296131: jge    0x180296151
00296133: test   rdx, rdx
00296136: je     0x1802962a7
0029613c: cmp    dword ptr [rdx + 0x18], 6
00296140: jbe    0x1802962ad
00296146: mov    rdx, qword ptr [rdx + 0x50]
0029614a: xor    esi, esi
0029614c: jmp    0x180296201
00296151: test   rdx, rdx
00296154: je     0x1802962a7
0029615a: cmp    dword ptr [rdx + 0x18], 5
0029615e: jbe    0x1802962ad
00296164: mov    rdx, qword ptr [rdx + 0x48]
00296168: jmp    0x180296201
0029616d: test   rdx, rdx
00296170: je     0x1802962a7
00296176: cmp    dword ptr [rdx + 0x18], 9
0029617a: jbe    0x1802962ad
00296180: mov    rdx, qword ptr [rdx + 0x68]
00296184: jmp    0x180296201
00296186: test   rdx, rdx
00296189: je     0x1802962a7
0029618f: cmp    dword ptr [rdx + 0x18], 4
00296193: jbe    0x1802962ad
00296199: mov    rdx, qword ptr [rdx + 0x40]
0029619d: jmp    0x180296201
0029619f: test   rdx, rdx
002961a2: je     0x1802962a7
002961a8: cmp    dword ptr [rdx + 0x18], 3
002961ac: jbe    0x1802962ad
002961b2: mov    rdx, qword ptr [rdx + 0x38]
002961b6: jmp    0x180296201
002961b8: test   rdx, rdx
002961bb: je     0x1802962a7
002961c1: cmp    dword ptr [rdx + 0x18], 2
002961c5: jbe    0x1802962ad
002961cb: mov    rdx, qword ptr [rdx + 0x30]
002961cf: jmp    0x180296201
002961d1: test   rdx, rdx
002961d4: je     0x1802962a7
002961da: cmp    dword ptr [rdx + 0x18], 1
002961de: jbe    0x1802962ad
002961e4: mov    rdx, qword ptr [rdx + 0x28]
002961e8: jmp    0x180296201
002961ea: test   rdx, rdx
002961ed: je     0x1802962a7
002961f3: cmp    dword ptr [rdx + 0x18], 0
002961f7: jbe    0x1802962ad
002961fd: mov    rdx, qword ptr [rdx + 0x20]
00296201: mov    rax, rdx
00296204: lea    rcx, [rdi + 0x78]
00296208: mov    qword ptr [rdi + 0x78], rdx
0029620c: call   0x182f5fc00
00296211: test   bpl, bpl
00296214: je     0x180296243
00296216: mov    rax, qword ptr [rip + 0x3a3bd1b]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
0029621d: mov    rcx, qword ptr [rax + 0xb8]
00296224: mov    rdx, qword ptr [rcx]
00296227: test   rdx, rdx
0029622a: je     0x1802962a7
0029622c: cmp    dword ptr [rdx + 0x18], 7
00296230: jbe    0x1802962ad
00296232: mov    rdx, qword ptr [rdx + 0x58]
00296236: lea    rcx, [rdi + 0x78]
0029623a: mov    qword ptr [rdi + 0x78], rdx
0029623e: call   0x182f5fc00
00296243: mov    rbx, qword ptr [rdi + 0x30]
00296247: mov    dword ptr [rdi + 0x80], esi
0029624d: cmp    byte ptr [rip + 0x3c7672b], 0            ; [0x3f0c97f] (bss)
00296254: jne    0x180296269
00296256: lea    rcx, [rip + 0x3a5b17b]                   ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
0029625d: call   0x182f609b0
00296262: mov    byte ptr [rip + 0x3c76716], 1            ; [0x3f0c97f] (bss)
00296269: test   rbx, rbx
0029626c: je     0x1802962a7
0029626e: mov    rax, qword ptr [rip + 0x3a5b163]         ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
00296275: lea    rdx, [rsp + 0x20]
0029627a: xor    r8d, r8d
0029627d: mov    rcx, rbx
00296280: mov    rax, qword ptr [rax + 0xb8]
00296287: movsd  xmm0, qword ptr [rax + 0xc]
0029628c: mov    eax, dword ptr [rax + 0x14]
0029628f: movsd  qword ptr [rsp + 0x20], xmm0
00296295: mov    dword ptr [rsp + 0x28], eax
00296299: call   0x18288a650                              ; UnityEngine.Transform$$set_localScale
0029629e: add    rsp, 0x38
002962a2: pop    rdi
002962a3: pop    rsi
002962a4: pop    rbp
002962a5: pop    rbx
002962a6: ret    
002962a7: call   0x182f60c50
002962ac: int3   
002962ad: call   0x182f60c40
002962b2: int3   
002962b3: int3   
