00294c60: mov    dword ptr [rsp + 0x10], edx
00294c64: push   rbx
00294c65: sub    rsp, 0x20
00294c69: cmp    byte ptr [rip + 0x3c78b94], 0            ; [0x3f0d804] (bss)
00294c70: mov    rbx, rcx
00294c73: jne    0x180294c8c
00294c75: lea    rcx, [rip + 0x3a3bb9c]                   ; [0x3cd0818] str:'%'
00294c7c: call   0x182f609b0
00294c81: mov    edx, dword ptr [rsp + 0x38]
00294c85: mov    byte ptr [rip + 0x3c78b78], 1            ; [0x3f0d804] (bss)
00294c8c: cmp    byte ptr [rbx + 0x50], 0
00294c90: jne    0x180294cfe
00294c92: cmp    edx, dword ptr [rbx + 0x84]
00294c98: je     0x180294cfe
00294c9a: mov    qword ptr [rsp + 0x30], rdi
00294c9f: mov    rdi, qword ptr [rbx + 0x28]
00294ca3: lea    rcx, [rsp + 0x38]
00294ca8: xor    edx, edx
00294caa: call   0x181b4f130                              ; System.Int32$$ToString
00294caf: mov    rdx, qword ptr [rip + 0x3a3bb62]         ; [0x3cd0818] str:'%'
00294cb6: xor    r8d, r8d
00294cb9: mov    rcx, rax
00294cbc: call   0x18197f200                              ; System.String$$Concat
00294cc1: test   rdi, rdi
00294cc4: je     0x180294d04
00294cc6: mov    r9, qword ptr [rdi]
00294cc9: mov    rdx, rax
00294ccc: mov    rcx, rdi
00294ccf: mov    r8, qword ptr [r9 + 0x5f0]
00294cd6: call   qword ptr [r9 + 0x5e8]
00294cdd: mov    eax, dword ptr [rsp + 0x38]
00294ce1: mov    rdi, qword ptr [rsp + 0x30]
00294ce6: mov    dword ptr [rbx + 0x84], eax
00294cec: mov    byte ptr [rbx + 0x51], 1
00294cf0: mov    dword ptr [rbx + 0x58], 0x3f933333
00294cf7: mov    dword ptr [rbx + 0x54], 0x3f933333
00294cfe: add    rsp, 0x20
00294d02: pop    rbx
00294d03: ret    
00294d04: call   0x182f60c50
00294d09: int3   
00294d0a: int3   
