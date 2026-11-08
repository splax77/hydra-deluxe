0216a3f0: sub    rsp, 0x28
0216a3f4: mov    rdx, qword ptr [rcx + 0x20]
0216a3f8: test   rdx, rdx
0216a3fb: je     0x18216a430
0216a3fd: cmp    byte ptr [rcx + 0x40], 0
0216a401: je     0x18216a41f
0216a403: mov    rax, qword ptr [rcx + 0x28]
0216a407: test   rax, rax
0216a40a: je     0x18216a430
0216a40c: movzx  eax, word ptr [rax + 0x80]
0216a413: or     ax, word ptr [rdx + 0x80]
0216a41a: add    rsp, 0x28
0216a41e: ret    
0216a41f: mov    eax, 1
0216a424: or     ax, word ptr [rdx + 0x80]
0216a42b: add    rsp, 0x28
0216a42f: ret    
0216a430: call   0x182f60c50
0216a435: int3   
0216a436: int3   
