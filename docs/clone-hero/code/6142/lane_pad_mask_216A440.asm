0216a440: sub    rsp, 0x28
0216a444: mov    rdx, qword ptr [rcx + 0x20]
0216a448: test   rdx, rdx
0216a44b: je     0x18216a47d
0216a44d: cmp    byte ptr [rcx + 0x40], 0
0216a451: je     0x18216a46f
0216a453: mov    rax, qword ptr [rcx + 0x28]
0216a457: test   rax, rax
0216a45a: je     0x18216a47d
0216a45c: movzx  eax, word ptr [rax + 0x80]
0216a463: or     ax, word ptr [rdx + 0x80]
0216a46a: add    rsp, 0x28
0216a46e: ret    
0216a46f: xor    eax, eax
0216a471: or     ax, word ptr [rdx + 0x80]
0216a478: add    rsp, 0x28
0216a47c: ret    
0216a47d: call   0x182f60c50
0216a482: int3   
0216a483: int3   
