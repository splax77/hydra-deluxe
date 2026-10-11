020de420: sub    rsp, 0x28
020de424: cmp    byte ptr [rcx + 0x2f1], 0
020de42b: je     0x1820de44c
020de42d: xor    eax, eax
020de42f: mov    dword ptr [rcx + 0x2ec], 0
020de439: mov    dword ptr [rcx + 0x2c4], eax
020de43f: mov    byte ptr [rcx + 0x2c8], al
020de445: mov    word ptr [rcx + 0x2f0], ax
020de44c: test   rdx, rdx
020de44f: je     0x1820de4b5
020de451: movzx  eax, word ptr [rcx + 0x2c4]
020de458: test   word ptr [rdx + 0x80], ax
020de45f: jne    0x1820de4b0
020de461: test   dword ptr [rdx + 0x7c], 0x8000
020de468: mov    byte ptr [rcx + 0x2f1], 0
020de46f: seta   al
020de472: mov    byte ptr [rcx + 0x2c8], al
020de478: movzx  eax, word ptr [rcx + 0x2c4]
020de47f: or     ax, word ptr [rdx + 0x80]
020de486: mov    word ptr [rcx + 0x2c4], ax
020de48d: movzx  eax, word ptr [rcx + 0x2c6]
020de494: or     ax, word ptr [rdx + 0x80]
020de49b: mov    word ptr [rcx + 0x2c6], ax
020de4a2: mov    rax, qword ptr [rcx + 0x100]
020de4a9: mov    qword ptr [rcx + 0x2e0], rax
020de4b0: add    rsp, 0x28
020de4b4: ret    
020de4b5: call   0x182f60c50
020de4ba: int3   
020de4bb: int3   
