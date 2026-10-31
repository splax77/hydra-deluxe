001c9ca0: mov    qword ptr [rsp + 0x10], rbx
001c9ca5: mov    qword ptr [rsp + 0x18], rsi
001c9caa: push   rdi
001c9cab: push   r14
001c9cad: push   r15
001c9caf: sub    rsp, 0x40
001c9cb3: mov    rdi, rcx
001c9cb6: cmp    byte ptr [rip + 0x3d42dcb], 0            ; [0x3f0ca88] (bss)
001c9cbd: jne    0x1801c9d5a
001c9cc3: lea    rcx, [rip + 0x3aec30e]                   ; [0x3cb5fd8] meta:UnityEngine.Application_TypeInfo
001c9cca: call   0x182f609b0
001c9ccf: lea    rcx, [rip + 0x3b33ffa]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
001c9cd6: call   0x182f609b0
001c9cdb: lea    rcx, [rip + 0x3b0690e]                   ; [0x3cd05f0] meta:System.Collections.Generic.IEnumerable<string>_TypeInfo
001c9ce2: call   0x182f609b0
001c9ce7: lea    rcx, [rip + 0x3b0d422]                   ; [0x3cd7110] meta:System.Collections.Generic.IEnumerator<string>_TypeInfo
001c9cee: call   0x182f609b0
001c9cf3: lea    rcx, [rip + 0x3b35506]                   ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
001c9cfa: call   0x182f609b0
001c9cff: lea    rcx, [rip + 0x3b0977a]                   ; [0x3cd3480] metamethod:Method$System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>..ctor()
001c9d06: call   0x182f609b0
001c9d0b: lea    rcx, [rip + 0x3b2b8fe]                   ; [0x3cf5610] meta:System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>_TypeInfo
001c9d12: call   0x182f609b0
001c9d17: lea    rcx, [rip + 0x3aed432]                   ; [0x3cb7150] meta:System.IO.Path_TypeInfo
001c9d1e: call   0x182f609b0
001c9d23: lea    rcx, [rip + 0x3b4423e]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001c9d2a: call   0x182f609b0
001c9d2f: lea    rcx, [rip + 0x3ace7d2]                   ; [0x3c98508] str:'profiles.ini'
001c9d36: call   0x182f609b0
001c9d3b: lea    rcx, [rip + 0x3ace64e]                   ; [0x3c98390] str:'profiles'
001c9d42: call   0x182f609b0
001c9d47: lea    rcx, [rip + 0x3ace702]                   ; [0x3c98450] str:'profiles.bin'
001c9d4e: call   0x182f609b0
001c9d53: mov    byte ptr [rip + 0x3d42d2e], 1            ; [0x3f0ca88] (bss)
001c9d5a: mov    qword ptr [rsp + 0x60], 0
001c9d63: mov    rcx, qword ptr [rip + 0x3b2b8a6]         ; [0x3cf5610] meta:System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>_TypeInfo
001c9d6a: call   0x182f60c00
001c9d6f: mov    rbx, rax
001c9d72: mov    rdx, qword ptr [rip + 0x3b09707]         ; [0x3cd3480] metamethod:Method$System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>..ctor()
001c9d79: mov    rcx, rax
001c9d7c: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
001c9d81: mov    qword ptr [rdi + 0x20], rbx
001c9d85: lea    rcx, [rdi + 0x20]
001c9d89: mov    rdx, rbx
001c9d8c: call   0x182f5fc00
001c9d91: mov    rcx, qword ptr [rip + 0x3aec240]         ; [0x3cb5fd8] meta:UnityEngine.Application_TypeInfo
001c9d98: cmp    dword ptr [rcx + 0xe0], 0
001c9d9f: jne    0x1801c9da6
001c9da1: call   0x182f60cf0
001c9da6: xor    ecx, ecx
001c9da8: call   0x182859290                              ; UnityEngine.Application$$get_persistentDataPath
001c9dad: mov    rbx, rax
001c9db0: mov    rcx, qword ptr [rip + 0x3aed399]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
001c9db7: cmp    dword ptr [rcx + 0xe0], 0
001c9dbe: jne    0x1801c9dc5
001c9dc0: call   0x182f60cf0
001c9dc5: xor    r8d, r8d
001c9dc8: mov    rdx, qword ptr [rip + 0x3ace739]         ; [0x3c98508] str:'profiles.ini'
001c9dcf: mov    rcx, rbx
001c9dd2: call   0x181b1eee0                              ; System.IO.Path$$Combine
001c9dd7: mov    rbx, rax
001c9dda: xor    ecx, ecx
001c9ddc: call   0x182859290                              ; UnityEngine.Application$$get_persistentDataPath
001c9de1: xor    r8d, r8d
001c9de4: mov    rdx, qword ptr [rip + 0x3ace665]         ; [0x3c98450] str:'profiles.bin'
001c9deb: mov    rcx, rax
001c9dee: call   0x181b1eee0                              ; System.IO.Path$$Combine
001c9df3: mov    rsi, rax
001c9df6: xor    edx, edx
001c9df8: mov    rcx, qword ptr [rdi + 0x18]
001c9dfc: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e01: xor    edx, edx
001c9e03: test   al, al
001c9e05: je     0x1801c9e21
001c9e07: mov    rcx, qword ptr [rdi + 0x10]
001c9e0b: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e10: test   al, al
001c9e12: je     0x1801c9e77
001c9e14: xor    edx, edx
001c9e16: mov    rcx, qword ptr [rdi + 0x10]
001c9e1a: call   0x181b1aa60                              ; System.IO.File$$Delete
001c9e1f: jmp    0x1801c9e77
001c9e21: mov    rcx, rbx
001c9e24: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e29: test   al, al
001c9e2b: je     0x1801c9e3c
001c9e2d: xor    r8d, r8d
001c9e30: mov    rdx, qword ptr [rdi + 0x18]
001c9e34: mov    rcx, rbx
001c9e37: call   0x181b1a8a0                              ; System.IO.File$$Copy
001c9e3c: xor    edx, edx
001c9e3e: mov    rcx, qword ptr [rdi + 0x18]
001c9e42: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e47: test   al, al
001c9e49: jne    0x1801c9e77
001c9e4b: xor    edx, edx
001c9e4d: mov    rcx, qword ptr [rdi + 0x10]
001c9e51: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e56: test   al, al
001c9e58: jne    0x1801c9e77
001c9e5a: xor    edx, edx
001c9e5c: mov    rcx, rsi
001c9e5f: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e64: test   al, al
001c9e66: je     0x1801c9e77
001c9e68: xor    r8d, r8d
001c9e6b: mov    rdx, qword ptr [rdi + 0x10]
001c9e6f: mov    rcx, rsi
001c9e72: call   0x181b1a8a0                              ; System.IO.File$$Copy
001c9e77: xor    edx, edx
001c9e79: mov    rcx, qword ptr [rdi + 0x18]
001c9e7d: call   0x181b1ab40                              ; System.IO.File$$Exists
001c9e82: test   al, al
001c9e84: je     0x1801ca0b7
001c9e8a: mov    rcx, qword ptr [rip + 0x3b440d7]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001c9e91: cmp    dword ptr [rcx + 0xe0], 0
001c9e98: jne    0x1801c9e9f
001c9e9a: call   0x182f60cf0
001c9e9f: xor    edx, edx
001c9ea1: mov    rcx, qword ptr [rip + 0x3ace4e8]         ; [0x3c98390] str:'profiles'
001c9ea8: call   0x1821150e0                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˀˁʷʶʶʴʾʴʽʵʶ
001c9ead: test   rax, rax
001c9eb0: je     0x1801ca10f
001c9eb6: xor    ecx, ecx
001c9eb8: mov    r8, rax
001c9ebb: mov    rdx, qword ptr [rip + 0x3b0672e]         ; [0x3cd05f0] meta:System.Collections.Generic.IEnumerable<string>_TypeInfo
001c9ec2: call   0x182c44210
001c9ec7: mov    qword ptr [rsp + 0x60], rax
001c9ecc: mov    qword ptr [rsp + 0x28], 0
001c9ed5: lea    rbx, [rsp + 0x60]
001c9eda: mov    qword ptr [rsp + 0x30], rbx
001c9edf: nop    
001c9ee0: mov    r8, qword ptr [rsp + 0x60]
001c9ee5: test   r8, r8
001c9ee8: je     0x1801ca103
001c9eee: xor    ecx, ecx
001c9ef0: mov    rdx, qword ptr [rip + 0x3b35309]         ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
001c9ef7: call   0x182c44210
001c9efc: test   al, al
001c9efe: je     0x1801ca053
001c9f04: mov    rsi, qword ptr [rsp + 0x60]
001c9f09: test   rsi, rsi
001c9f0c: je     0x1801ca0fe
001c9f12: mov    r9, qword ptr [rip + 0x3b0d1f7]          ; [0x3cd7110] meta:System.Collections.Generic.IEnumerator<string>_TypeInfo
001c9f19: mov    r10, qword ptr [rsi]
001c9f1c: movzx  edx, word ptr [r10 + 0x12e]
001c9f24: xor    eax, eax
001c9f26: cmp    ax, dx
001c9f29: jae    0x1801c9f58
001c9f2b: xor    ecx, ecx
001c9f2d: mov    r8, qword ptr [r10 + 0xb0]
001c9f34: nop    dword ptr [rax]
001c9f38: nop    dword ptr [rax + rax]
001c9f40: movzx  eax, cx
001c9f43: add    rax, rax
001c9f46: cmp    qword ptr [r8 + rax*8], r9
001c9f4a: je     0x1801ca036
001c9f50: inc    cx
001c9f53: cmp    cx, dx
001c9f56: jb     0x1801c9f40
001c9f58: xor    r8d, r8d
001c9f5b: mov    rdx, r9
001c9f5e: mov    rcx, rsi
001c9f61: call   0x182f65070
001c9f66: mov    rdx, qword ptr [rax + 8]
001c9f6a: mov    rcx, rsi
001c9f6d: call   qword ptr [rax]
001c9f6f: test   rax, rax
001c9f72: je     0x1801ca0f9
001c9f78: xor    r8d, r8d
001c9f7b: mov    edx, 7
001c9f80: mov    rcx, rax
001c9f83: call   0x181988940                              ; System.String$$Substring
001c9f88: xor    edx, edx
001c9f8a: mov    rcx, rax
001c9f8d: call   0x181b4e820                              ; System.Int32$$Parse
001c9f92: mov    r14d, eax
001c9f95: cmp    byte ptr [rip + 0x3d42af8], 0            ; [0x3f0ca94] (bss)
001c9f9c: jne    0x1801c9fd5
001c9f9e: lea    rcx, [rip + 0x3b0959b]                   ; [0x3cd3540] metamethod:Method$System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>.Add()
001c9fa5: call   0x182f609b0
001c9faa: lea    rcx, [rip + 0x3ace4e7]                   ; [0x3c98498] meta:ʹʺʿʻʲʹʻʹʹʵʸ_TypeInfo
001c9fb1: call   0x182f609b0
001c9fb6: lea    rcx, [rip + 0x3ace3d3]                   ; [0x3c98390] str:'profiles'
001c9fbd: call   0x182f609b0
001c9fc2: lea    rcx, [rip + 0x3ae9ea7]                   ; [0x3cb3e70] str:''
001c9fc9: call   0x182f609b0
001c9fce: mov    byte ptr [rip + 0x3d42abf], 1            ; [0x3f0ca94] (bss)
001c9fd5: mov    rsi, qword ptr [rip + 0x3ace3b4]         ; [0x3c98390] str:'profiles'
001c9fdc: cmp    r14d, -1
001c9fe0: cmove  rsi, qword ptr [rip + 0x3ae9e88]         ; [0x3cb3e70] str:''
001c9fe8: mov    rcx, qword ptr [rip + 0x3ace4a9]         ; [0x3c98498] meta:ʹʺʿʻʲʹʻʹʹʵʸ_TypeInfo
001c9fef: call   0x182f60c00
001c9ff4: mov    r15, rax
001c9ff7: xor    r9d, r9d
001c9ffa: mov    r8, rsi
001c9ffd: mov    edx, r14d
001ca000: mov    rcx, rax
001ca003: call   0x1800d8a90                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$.ctor
001ca008: cmp    r14d, -1
001ca00c: je     0x1801c9ee0
001ca012: mov    rcx, qword ptr [rdi + 0x20]
001ca016: test   rcx, rcx
001ca019: je     0x1801ca0f3
001ca01f: mov    r8, qword ptr [rip + 0x3b0951a]          ; [0x3cd3540] metamethod:Method$System.Collections.Generic.List<ʹʺʿʻʲʹʻʹʹʵʸ>.Add()
001ca026: mov    rdx, r15
001ca029: call   0x182c425f0
001ca02e: inc    dword ptr [rdi + 0x28]
001ca031: jmp    0x1801c9ee0
001ca036: movzx  ecx, cx
001ca039: add    rcx, rcx
001ca03c: movsxd rax, dword ptr [r8 + rcx*8 + 8]
001ca041: shl    rax, 4
001ca045: add    rax, 0x138
001ca04b: add    rax, r10
001ca04e: jmp    0x1801c9f66
001ca053: cmp    qword ptr [rbx], 0
001ca057: je     0x1801ca06a
001ca059: xor    ecx, ecx
001ca05b: mov    r8, qword ptr [rbx]
001ca05e: mov    rdx, qword ptr [rip + 0x3b33c6b]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
001ca065: call   0x182c42550
001ca06a: jmp    0x1801ca092
001ca06c: mov    r8, qword ptr [rsp + 0x30]
001ca071: cmp    qword ptr [r8], 0
001ca075: je     0x1801ca088
001ca077: xor    ecx, ecx
001ca079: mov    r8, qword ptr [r8]
001ca07c: mov    rdx, qword ptr [rip + 0x3b33c4d]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
001ca083: call   0x182c42550
001ca088: mov    rcx, qword ptr [rsp + 0x28]
001ca08d: test   rcx, rcx
001ca090: jne    0x1801ca109
001ca092: mov    rcx, qword ptr [rip + 0x3b43ecf]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001ca099: cmp    dword ptr [rcx + 0xe0], 0
001ca0a0: jne    0x1801ca0a7
001ca0a2: call   0x182f60cf0
001ca0a7: xor    edx, edx
001ca0a9: mov    rcx, qword ptr [rip + 0x3ace2e0]         ; [0x3c98390] str:'profiles'
001ca0b0: call   0x182114a80                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˀʷʳʵʸʼˀʷʵʷʶ
001ca0b5: jmp    0x1801ca0df
001ca0b7: xor    edx, edx
001ca0b9: mov    rcx, rdi
001ca0bc: call   0x1801cb070                              ; ʷʾʺʾʺʲʿʲˀʲˀ$$ˁʹʸʻʸʲʶʲʶˁʳ
001ca0c1: mov    rcx, qword ptr [rip + 0x3b43ea0]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001ca0c8: cmp    dword ptr [rcx + 0xe0], 0
001ca0cf: jne    0x1801ca0d6
001ca0d1: call   0x182f60cf0
001ca0d6: xor    edx, edx
001ca0d8: xor    ecx, ecx
001ca0da: call   0x182112db0                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʷʶʿʲʾʽʷʺʳʲʵ
001ca0df: mov    rbx, qword ptr [rsp + 0x68]
001ca0e4: mov    rsi, qword ptr [rsp + 0x70]
001ca0e9: add    rsp, 0x40
001ca0ed: pop    r15
001ca0ef: pop    r14
001ca0f1: pop    rdi
001ca0f2: ret    
001ca0f3: call   0x182f60c50
001ca0f8: nop    
001ca0f9: call   0x182f60c50
001ca0fe: call   0x182f60c50
001ca103: call   0x182f60c50
001ca108: nop    
001ca109: call   0x182f60ce0
001ca10e: int3   
001ca10f: call   0x182f60c50
001ca114: int3   
001ca115: int3   
