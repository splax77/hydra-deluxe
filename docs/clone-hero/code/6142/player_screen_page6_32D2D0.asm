0032d2d0: push   rbx
0032d2d2: sub    rsp, 0x20
0032d2d6: mov    rax, qword ptr [rcx + 0x20]
0032d2da: mov    rbx, rcx
0032d2dd: mov    dword ptr [rcx + 0x120], 6
0032d2e7: test   rax, rax
0032d2ea: je     0x18032d34d
0032d2ec: mov    rcx, qword ptr [rax + 0x10]
0032d2f0: test   rcx, rcx
0032d2f3: je     0x18032d34d
0032d2f5: movzx  r8d, byte ptr [rbx + 0x241]
0032d2fd: xor    r9d, r9d
0032d300: movzx  edx, byte ptr [rbx + 0x240]
0032d307: call   0x1800d86d0                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʾʵʽʳʳˁʼʼʾʶʷ
0032d30c: mov    rcx, qword ptr [rbx + 0x158]
0032d313: test   rcx, rcx
0032d316: je     0x18032d34d
0032d318: cmp    dword ptr [rbx + 0x120], 5
0032d31f: sete   dl
0032d322: xor    r8d, r8d
0032d325: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0032d32a: mov    rcx, qword ptr [rbx + 0x160]
0032d331: test   rcx, rcx
0032d334: je     0x18032d34d
0032d336: cmp    dword ptr [rbx + 0x120], 7
0032d33d: sete   dl
0032d340: xor    r8d, r8d
0032d343: add    rsp, 0x20
0032d347: pop    rbx
0032d348: jmp    0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0032d34d: call   0x182f60c50
0032d352: int3   
0032d353: int3   
