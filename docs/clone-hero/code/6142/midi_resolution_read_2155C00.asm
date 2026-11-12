02155c00: push   rbx
02155c02: push   r14
02155c04: sub    rsp, 0x38
02155c08: cmp    byte ptr [rip + 0x1dc3a86], 0            ; [0x3f19695] (bss)
02155c0f: mov    r14, rdx
02155c12: mov    rbx, rcx
02155c15: jne    0x182155d36
02155c1b: lea    rcx, [rip + 0x1b88eee]                   ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
02155c22: call   0x182f609b0
02155c27: lea    rcx, [rip + 0x1b88e22]                   ; [0x3cdea50] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>..ctor()
02155c2e: call   0x182f609b0
02155c33: lea    rcx, [rip + 0x1b7029e]                   ; [0x3cc5ed8] meta:System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>_TypeInfo
02155c3a: call   0x182f609b0
02155c3f: lea    rcx, [rip + 0x1b82d9a]                   ; [0x3cd89e0] metamethod:Method$System.Collections.Generic.List<ˀʽʵʲˁʴˀʴʽʷʳ>.get_Count()
02155c46: call   0x182f609b0
02155c4b: lea    rcx, [rip + 0x1b82e4e]                   ; [0x3cd8aa0] metamethod:Method$System.Collections.Generic.List<ˀʽʵʲˁʴˀʴʽʷʳ>.get_Item()
02155c52: call   0x182f609b0
02155c57: lea    rcx, [rip + 0x1b76dea]                   ; [0x3ccca48] metamethod:Method$System.Threading.ThreadLocal<bool>.set_Value()
02155c5e: call   0x182f609b0
02155c63: lea    rcx, [rip + 0x1b3e6d6]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155c6a: call   0x182f609b0
02155c6f: lea    rcx, [rip + 0x1badef2]                   ; [0x3d03b68] str:'part bass'
02155c76: call   0x182f609b0
02155c7b: lea    rcx, [rip + 0x1badf9e]                   ; [0x3d03c20] str:'part bass ghl'
02155c82: call   0x182f609b0
02155c87: lea    rcx, [rip + 0x1b7e45a]                   ; [0x3cd40e8] str:'t1 gems'
02155c8e: call   0x182f609b0
02155c93: lea    rcx, [rip + 0x1bae3ee]                   ; [0x3d04088] str:'part guitar ghl'
02155c9a: call   0x182f609b0
02155c9f: lea    rcx, [rip + 0x1b9f2a2]                   ; [0x3cf4f48] str:'events'
02155ca6: call   0x182f609b0
02155cab: lea    rcx, [rip + 0x1bae1a6]                   ; [0x3d03e58] str:'part guitar'
02155cb2: call   0x182f609b0
02155cb7: lea    rcx, [rip + 0x1bae312]                   ; [0x3d03fd0] str:'part guitar coop ghl'
02155cbe: call   0x182f609b0
02155cc3: lea    rcx, [rip + 0x1b9aa3e]                   ; [0x3cf0708] str:'venue'
02155cca: call   0x182f609b0
02155ccf: lea    rcx, [rip + 0x1bae6a2]                   ; [0x3d04378] str:'part rhythm ghl'
02155cd6: call   0x182f609b0
02155cdb: lea    rcx, [rip + 0x1bae22e]                   ; [0x3d03f10] str:'part guitar coop'
02155ce2: call   0x182f609b0
02155ce7: lea    rcx, [rip + 0x1bae0aa]                   ; [0x3d03d98] str:'part drums'
02155cee: call   0x182f609b0
02155cf3: lea    rcx, [rip + 0x1bae506]                   ; [0x3d04200] str:'part keys ghl'
02155cfa: call   0x182f609b0
02155cff: lea    rcx, [rip + 0x1bae732]                   ; [0x3d04438] str:'part vocals'
02155d06: call   0x182f609b0
02155d0b: lea    rcx, [rip + 0x1bae5ae]                   ; [0x3d042c0] str:'part rhythm'
02155d12: call   0x182f609b0
02155d17: lea    rcx, [rip + 0x1bae42a]                   ; [0x3d04148] str:'part keys'
02155d1e: call   0x182f609b0
02155d23: lea    rcx, [rip + 0x1badfb6]                   ; [0x3d03ce0] str:'part drum'
02155d2a: call   0x182f609b0
02155d2f: mov    byte ptr [rip + 0x1dc395f], 1            ; [0x3f19695] (bss)
02155d36: mov    rax, qword ptr [rip + 0x1b3e603]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155d3d: cmp    dword ptr [rax + 0xe0], 0
02155d44: jne    0x182155d55
02155d46: mov    rcx, rax
02155d49: call   0x182f60cf0
02155d4e: mov    rax, qword ptr [rip + 0x1b3e5eb]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155d55: mov    rdx, qword ptr [r14]
02155d58: mov    qword ptr [rsp + 0x50], rbp
02155d5d: mov    qword ptr [rsp + 0x58], rsi
02155d62: mov    qword ptr [rsp + 0x60], rdi
02155d67: mov    qword ptr [rsp + 0x30], r12
02155d6c: mov    qword ptr [rsp + 0x28], r13
02155d71: mov    qword ptr [rsp + 0x20], r15
02155d76: test   rdx, rdx
02155d79: je     0x182156715
02155d7f: mov    rax, qword ptr [rax + 0xb8]
02155d86: mov    rcx, qword ptr [rax]
02155d89: test   rcx, rcx
02155d8c: je     0x182156715
02155d92: mov    r8, qword ptr [rip + 0x1b76caf]          ; [0x3ccca48] metamethod:Method$System.Threading.ThreadLocal<bool>.set_Value()
02155d99: movzx  edx, byte ptr [rdx + 0x2c]
02155d9d: call   0x180b0bbf0                              ; System.Threading.ThreadLocal<bool>$$set_Value
02155da2: mov    rcx, qword ptr [r14]
02155da5: test   rcx, rcx
02155da8: je     0x182156715
02155dae: test   rbx, rbx
02155db1: je     0x182156715
02155db7: mov    rax, qword ptr [rbx + 0xf0]
02155dbe: test   rax, rax
02155dc1: je     0x182156715
02155dc7: mov    r13, qword ptr [rcx + 0x30]
02155dcb: test   r13, r13
02155dce: je     0x182156715
02155dd4: movzx  eax, word ptr [rax + 0x12]
02155dd8: xor    r8d, r8d
02155ddb: mov    rbp, qword ptr [rbx + 0xf8]
02155de2: mov    rcx, r13
02155de5: movd   xmm1, eax
02155de9: cvtdq2pd xmm1, xmm1
02155ded: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
02155df2: mov    rcx, qword ptr [r14]
02155df5: test   rcx, rcx
02155df8: je     0x182156715
02155dfe: mov    rcx, qword ptr [rcx + 0x30]
02155e02: test   rcx, rcx
02155e05: je     0x182156715
02155e0b: mov    rax, qword ptr [r14]
02155e0e: movsd  xmm1, qword ptr [rip + 0xf0ebe2]         ; [0x30649f8] dbl=3.0 q=0x4008000000000000
02155e16: cmp    dword ptr [rax + 0x18], 0
02155e1a: jg     0x182155e46
02155e1c: cmp    byte ptr [rax + 0x1c], 0
02155e20: movsd  xmm0, qword ptr [rcx + 0xb8]
02155e28: jne    0x182155e36
02155e2a: divsd  xmm0, xmm1
02155e2e: cvttsd2si edx, xmm0
02155e32: inc    edx
02155e34: jmp    0x182155e49
02155e36: mulsd  xmm0, qword ptr [rip + 0xf0e172]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
02155e3e: cvttsd2si edx, xmm0
02155e42: inc    edx
02155e44: jmp    0x182155e49
02155e46: mov    edx, dword ptr [rax + 0x18]
02155e49: mov    rcx, qword ptr [rax + 0x30]
02155e4d: mov    dword ptr [rcx + 0x118], edx
02155e53: mov    rax, qword ptr [r14]
02155e56: test   rax, rax
02155e59: je     0x182156715
02155e5f: mov    rax, qword ptr [rax + 0x30]
02155e63: test   rax, rax
02155e66: je     0x182156715
02155e6c: mov    byte ptr [rax + 0x11c], 1
02155e73: mov    rcx, qword ptr [r14]
02155e76: test   rcx, rcx
02155e79: je     0x182156715
02155e7f: mov    rcx, qword ptr [rcx + 0x30]
02155e83: test   rcx, rcx
02155e86: je     0x182156715
02155e8c: mov    rax, qword ptr [r14]
02155e8f: cmp    dword ptr [rax + 0x24], 0
02155e93: jge    0x182155ea9
02155e95: movsd  xmm0, qword ptr [rcx + 0xb8]
02155e9d: divsd  xmm0, xmm1
02155ea1: cvttsd2si eax, xmm0
02155ea5: inc    eax
02155ea7: jmp    0x182155eac
02155ea9: mov    eax, dword ptr [rax + 0x24]
02155eac: mov    dword ptr [rcx + 0x114], eax
02155eb2: test   rbp, rbp
02155eb5: je     0x182156715
02155ebb: mov    r8, qword ptr [rip + 0x1b82bde]          ; [0x3cd8aa0] metamethod:Method$System.Collections.Generic.List<ˀʽʵʲˁʴˀʴʽʷʳ>.get_Item()
02155ec2: xor    edx, edx
02155ec4: mov    rcx, rbp
02155ec7: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
02155ecc: test   rax, rax
02155ecf: je     0x182156715
02155ed5: mov    rcx, qword ptr [rax + 0x18]
02155ed9: xor    r8d, r8d
02155edc: mov    rdx, r14
02155edf: call   0x182156720                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʸˁʿʿʾʾʷʶʻʻʷ
02155ee4: xor    ebx, ebx
02155ee6: xor    eax, eax
02155ee8: cmp    eax, dword ptr [rbp + 0x18]
02155eeb: jge    0x182155f6c
02155eed: mov    r8, qword ptr [rip + 0x1b82bac]          ; [0x3cd8aa0] metamethod:Method$System.Collections.Generic.List<ˀʽʵʲˁʴˀʴʽʷʳ>.get_Item()
02155ef4: mov    edx, ebx
02155ef6: mov    rcx, rbp
02155ef9: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
02155efe: mov    rdi, rax
02155f01: test   rax, rax
02155f04: je     0x182156715
02155f0a: mov    rcx, qword ptr [rax + 0x10]
02155f0e: xor    edx, edx
02155f10: call   0x181983d80                              ; System.String$$IsNullOrEmpty
02155f15: test   al, al
02155f17: jne    0x182155f3f
02155f19: mov    rcx, qword ptr [rdi + 0x10]
02155f1d: test   rcx, rcx
02155f20: je     0x182156715
02155f26: mov    rdx, qword ptr [rip + 0x1b9f01b]         ; [0x3cf4f48] str:'events'
02155f2d: xor    r9d, r9d
02155f30: mov    r8d, 5
02155f36: call   0x181981640                              ; System.String$$Equals
02155f3b: test   al, al
02155f3d: jne    0x182155f45
02155f3f: inc    ebx
02155f41: mov    eax, ebx
02155f43: jmp    0x182155ee8
02155f45: mov    rcx, qword ptr [rip + 0x1b3e3f4]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155f4c: mov    rbx, qword ptr [rdi + 0x18]
02155f50: cmp    dword ptr [rcx + 0xe0], 0
02155f57: jne    0x182155f5e
02155f59: call   0x182f60cf0
02155f5e: xor    r8d, r8d
02155f61: mov    rdx, r14
02155f64: mov    rcx, rbx
02155f67: call   0x1821544e0                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʲʼʵˁˁʲʳʻˁʷʻ
02155f6c: mov    rcx, qword ptr [rip + 0x1b6ff65]         ; [0x3cc5ed8] meta:System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>_TypeInfo
02155f73: call   0x182f60c00
02155f78: mov    rdx, qword ptr [rip + 0x1b88ad1]         ; [0x3cdea50] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>..ctor()
02155f7f: mov    rcx, rax
02155f82: mov    rsi, rax
02155f85: call   0x18174cab0                              ; System.Collections.Generic.HashSet<SByteEnum>$$.ctor
02155f8a: xor    r12b, r12b
02155f8d: mov    r15d, 1
02155f93: mov    ecx, r15d
02155f96: cmp    ecx, dword ptr [rbp + 0x18]
02155f99: jge    0x18215666d
02155f9f: mov    r8, qword ptr [rip + 0x1b82afa]          ; [0x3cd8aa0] metamethod:Method$System.Collections.Generic.List<ˀʽʵʲˁʴˀʴʽʷʳ>.get_Item()
02155fa6: mov    edx, r15d
02155fa9: mov    rcx, rbp
02155fac: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
02155fb1: mov    rdi, rax
02155fb4: test   rax, rax
02155fb7: je     0x182156715
02155fbd: mov    rcx, qword ptr [rax + 0x10]
02155fc1: xor    edx, edx
02155fc3: call   0x181983d80                              ; System.String$$IsNullOrEmpty
02155fc8: test   al, al
02155fca: jne    0x182156665
02155fd0: mov    rcx, qword ptr [rdi + 0x10]
02155fd4: test   rcx, rcx
02155fd7: je     0x182156715
02155fdd: xor    edx, edx
02155fdf: call   0x181989310                              ; System.String$$ToLowerInvariant
02155fe4: xor    edx, edx
02155fe6: mov    rcx, rax
02155fe9: mov    rbx, rax
02155fec: call   0x180103fe0                              ; <PrivateImplementationDetails>$$ComputeStringHash
02155ff1: cmp    eax, 0x8ad38d63
02155ff6: jbe    0x182156324
02155ffc: cmp    eax, 0xbaf52189
02156001: jbe    0x182156156
02156007: cmp    eax, 0xd8f48395
0215600c: jbe    0x1821560b4
02156012: cmp    eax, 0xd90948b6
02156017: jne    0x18215603b
02156019: mov    rdx, qword ptr [rip + 0x1bade38]         ; [0x3d03e58] str:'part guitar'
02156020: xor    r8d, r8d
02156023: mov    rcx, rbx
02156026: call   0x181981570                              ; System.String$$op_Equality
0215602b: test   al, al
0215602d: jne    0x18215635b
02156033: inc    r15d
02156036: jmp    0x182155f93
0215603b: cmp    eax, 0xf1a7745d
02156040: jne    0x182156665
02156046: mov    rdx, qword ptr [rip + 0x1bae32b]         ; [0x3d04378] str:'part rhythm ghl'
0215604d: xor    r8d, r8d
02156050: mov    rcx, rbx
02156053: call   0x181981570                              ; System.String$$op_Equality
02156058: test   al, al
0215605a: je     0x182156665
02156060: test   rsi, rsi
02156063: je     0x182156715
02156069: mov    r8, qword ptr [rip + 0x1b88aa0]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
02156070: mov    dl, 0xa
02156072: mov    rcx, rsi
02156075: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
0215607a: test   al, al
0215607c: je     0x182156665
02156082: mov    rcx, qword ptr [rip + 0x1b3e2b7]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156089: mov    rbx, qword ptr [rdi + 0x18]
0215608d: cmp    dword ptr [rcx + 0xe0], 0
02156094: jne    0x18215609b
02156096: call   0x182f60cf0
0215609b: xor    r9d, r9d
0215609e: mov    r8b, 0xa
021560a1: mov    rdx, r14
021560a4: mov    rcx, rbx
021560a7: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
021560ac: inc    r15d
021560af: jmp    0x182155f93
021560b4: cmp    eax, 0xc03a2b7f
021560b9: jne    0x182156129
021560bb: mov    rdx, qword ptr [rip + 0x1badaa6]         ; [0x3d03b68] str:'part bass'
021560c2: xor    r8d, r8d
021560c5: mov    rcx, rbx
021560c8: call   0x181981570                              ; System.String$$op_Equality
021560cd: test   al, al
021560cf: je     0x182156665
021560d5: test   rsi, rsi
021560d8: je     0x182156715
021560de: mov    r8, qword ptr [rip + 0x1b88a2b]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021560e5: mov    dl, 1
021560e7: mov    rcx, rsi
021560ea: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021560ef: test   al, al
021560f1: je     0x182156665
021560f7: mov    rcx, qword ptr [rip + 0x1b3e242]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021560fe: mov    rbx, qword ptr [rdi + 0x18]
02156102: cmp    dword ptr [rcx + 0xe0], 0
02156109: jne    0x182156110
0215610b: call   0x182f60cf0
02156110: xor    r9d, r9d
02156113: mov    r8b, 1
02156116: mov    rdx, r14
02156119: mov    rcx, rbx
0215611c: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
02156121: inc    r15d
02156124: jmp    0x182155f93
02156129: cmp    eax, 0xd8f48395
0215612e: jne    0x182156665
02156134: mov    rdx, qword ptr [rip + 0x1badc5d]         ; [0x3d03d98] str:'part drums'
0215613b: xor    r8d, r8d
0215613e: mov    rcx, rbx
02156141: call   0x181981570                              ; System.String$$op_Equality
02156146: test   al, al
02156148: jne    0x1821564c2
0215614e: inc    r15d
02156151: jmp    0x182155f93
02156156: cmp    eax, 0x9aa7273b
0215615b: jbe    0x182156236
02156161: cmp    eax, 0xb39aa8fc
02156166: jne    0x1821561bd
02156168: mov    rdx, qword ptr [rip + 0x1b9a599]         ; [0x3cf0708] str:'venue'
0215616f: xor    r8d, r8d
02156172: mov    rcx, rbx
02156175: call   0x181981570                              ; System.String$$op_Equality
0215617a: test   al, al
0215617c: je     0x182156665
02156182: test   r12b, r12b
02156185: jne    0x182156665
0215618b: mov    rcx, qword ptr [rip + 0x1b3e1ae]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156192: mov    r12b, 1
02156195: mov    rbx, qword ptr [rdi + 0x18]
02156199: cmp    dword ptr [rcx + 0xe0], 0
021561a0: jne    0x1821561a7
021561a2: call   0x182f60cf0
021561a7: xor    r8d, r8d
021561aa: mov    rdx, r14
021561ad: mov    rcx, rbx
021561b0: call   0x182154c30                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʲˁʵʳʷʹˁʻʶʳˀ
021561b5: inc    r15d
021561b8: jmp    0x182155f93
021561bd: cmp    eax, 0xbaf52189
021561c2: jne    0x182156665
021561c8: mov    rdx, qword ptr [rip + 0x1bae031]         ; [0x3d04200] str:'part keys ghl'
021561cf: xor    r8d, r8d
021561d2: mov    rcx, rbx
021561d5: call   0x181981570                              ; System.String$$op_Equality
021561da: test   al, al
021561dc: je     0x182156665
021561e2: test   rsi, rsi
021561e5: je     0x182156715
021561eb: mov    r8, qword ptr [rip + 0x1b8891e]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021561f2: mov    dl, 0xe
021561f4: mov    rcx, rsi
021561f7: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021561fc: test   al, al
021561fe: je     0x182156665
02156204: mov    rcx, qword ptr [rip + 0x1b3e135]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215620b: mov    rbx, qword ptr [rdi + 0x18]
0215620f: cmp    dword ptr [rcx + 0xe0], 0
02156216: jne    0x18215621d
02156218: call   0x182f60cf0
0215621d: xor    r9d, r9d
02156220: mov    r8b, 0xe
02156223: mov    rdx, r14
02156226: mov    rcx, rbx
02156229: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
0215622e: inc    r15d
02156231: jmp    0x182155f93
02156236: cmp    eax, 0x8e49ee2a
0215623b: jne    0x1821562ab
0215623d: mov    rdx, qword ptr [rip + 0x1bad9dc]         ; [0x3d03c20] str:'part bass ghl'
02156244: xor    r8d, r8d
02156247: mov    rcx, rbx
0215624a: call   0x181981570                              ; System.String$$op_Equality
0215624f: test   al, al
02156251: je     0x182156665
02156257: test   rsi, rsi
0215625a: je     0x182156715
02156260: mov    r8, qword ptr [rip + 0x1b888a9]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
02156267: mov    dl, 5
02156269: mov    rcx, rsi
0215626c: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
02156271: test   al, al
02156273: je     0x182156665
02156279: mov    rcx, qword ptr [rip + 0x1b3e0c0]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156280: mov    rbx, qword ptr [rdi + 0x18]
02156284: cmp    dword ptr [rcx + 0xe0], 0
0215628b: jne    0x182156292
0215628d: call   0x182f60cf0
02156292: xor    r9d, r9d
02156295: mov    r8b, 5
02156298: mov    rdx, r14
0215629b: mov    rcx, rbx
0215629e: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
021562a3: inc    r15d
021562a6: jmp    0x182155f93
021562ab: cmp    eax, 0x9aa7273b
021562b0: jne    0x182156665
021562b6: mov    rdx, qword ptr [rip + 0x1baddcb]         ; [0x3d04088] str:'part guitar ghl'
021562bd: xor    r8d, r8d
021562c0: mov    rcx, rbx
021562c3: call   0x181981570                              ; System.String$$op_Equality
021562c8: test   al, al
021562ca: je     0x182156665
021562d0: test   rsi, rsi
021562d3: je     0x182156715
021562d9: mov    r8, qword ptr [rip + 0x1b88830]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021562e0: mov    dl, 4
021562e2: mov    rcx, rsi
021562e5: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021562ea: test   al, al
021562ec: je     0x182156665
021562f2: mov    rcx, qword ptr [rip + 0x1b3e047]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021562f9: mov    rbx, qword ptr [rdi + 0x18]
021562fd: cmp    dword ptr [rcx + 0xe0], 0
02156304: jne    0x18215630b
02156306: call   0x182f60cf0
0215630b: xor    r9d, r9d
0215630e: mov    r8b, 4
02156311: mov    rdx, r14
02156314: mov    rcx, rbx
02156317: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
0215631c: inc    r15d
0215631f: jmp    0x182155f93
02156324: cmp    eax, 0x39c8f25a
02156329: jbe    0x182156516
0215632f: cmp    eax, 0x56cb3f44
02156334: jbe    0x182156428
0215633a: cmp    eax, 0x7f37d93e
0215633f: jne    0x1821563af
02156341: mov    rdx, qword ptr [rip + 0x1b7dda0]         ; [0x3cd40e8] str:'t1 gems'
02156348: xor    r8d, r8d
0215634b: mov    rcx, rbx
0215634e: call   0x181981570                              ; System.String$$op_Equality
02156353: test   al, al
02156355: je     0x182156665
0215635b: test   rsi, rsi
0215635e: je     0x182156715
02156364: mov    r8, qword ptr [rip + 0x1b887a5]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
0215636b: xor    edx, edx
0215636d: mov    rcx, rsi
02156370: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
02156375: test   al, al
02156377: je     0x182156665
0215637d: mov    rcx, qword ptr [rip + 0x1b3dfbc]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156384: mov    rbx, qword ptr [rdi + 0x18]
02156388: cmp    dword ptr [rcx + 0xe0], 0
0215638f: jne    0x182156396
02156391: call   0x182f60cf0
02156396: xor    r9d, r9d
02156399: xor    r8d, r8d
0215639c: mov    rdx, r14
0215639f: mov    rcx, rbx
021563a2: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
021563a7: inc    r15d
021563aa: jmp    0x182155f93
021563af: cmp    eax, 0x8ad38d63
021563b4: jne    0x182156665
021563ba: mov    rdx, qword ptr [rip + 0x1badb4f]         ; [0x3d03f10] str:'part guitar coop'
021563c1: xor    r8d, r8d
021563c4: mov    rcx, rbx
021563c7: call   0x181981570                              ; System.String$$op_Equality
021563cc: test   al, al
021563ce: je     0x182156665
021563d4: test   rsi, rsi
021563d7: je     0x182156715
021563dd: mov    r8, qword ptr [rip + 0x1b8872c]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021563e4: mov    dl, 3
021563e6: mov    rcx, rsi
021563e9: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021563ee: test   al, al
021563f0: je     0x182156665
021563f6: mov    rcx, qword ptr [rip + 0x1b3df43]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021563fd: mov    rbx, qword ptr [rdi + 0x18]
02156401: cmp    dword ptr [rcx + 0xe0], 0
02156408: jne    0x18215640f
0215640a: call   0x182f60cf0
0215640f: xor    r9d, r9d
02156412: mov    r8b, 3
02156415: mov    rdx, r14
02156418: mov    rcx, rbx
0215641b: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
02156420: inc    r15d
02156423: jmp    0x182155f93
02156428: cmp    eax, 0x4e5f73cc
0215642d: jne    0x18215649d
0215642f: mov    rdx, qword ptr [rip + 0x1bade8a]         ; [0x3d042c0] str:'part rhythm'
02156436: xor    r8d, r8d
02156439: mov    rcx, rbx
0215643c: call   0x181981570                              ; System.String$$op_Equality
02156441: test   al, al
02156443: je     0x182156665
02156449: test   rsi, rsi
0215644c: je     0x182156715
02156452: mov    r8, qword ptr [rip + 0x1b886b7]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
02156459: mov    dl, 2
0215645b: mov    rcx, rsi
0215645e: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
02156463: test   al, al
02156465: je     0x182156665
0215646b: mov    rcx, qword ptr [rip + 0x1b3dece]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156472: mov    rbx, qword ptr [rdi + 0x18]
02156476: cmp    dword ptr [rcx + 0xe0], 0
0215647d: jne    0x182156484
0215647f: call   0x182f60cf0
02156484: xor    r9d, r9d
02156487: mov    r8b, 2
0215648a: mov    rdx, r14
0215648d: mov    rcx, rbx
02156490: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
02156495: inc    r15d
02156498: jmp    0x182155f93
0215649d: cmp    eax, 0x56cb3f44
021564a2: jne    0x182156665
021564a8: mov    rdx, qword ptr [rip + 0x1bad831]         ; [0x3d03ce0] str:'part drum'
021564af: xor    r8d, r8d
021564b2: mov    rcx, rbx
021564b5: call   0x181981570                              ; System.String$$op_Equality
021564ba: test   al, al
021564bc: je     0x182156665
021564c2: test   rsi, rsi
021564c5: je     0x182156715
021564cb: mov    r8, qword ptr [rip + 0x1b8863e]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021564d2: mov    dl, 6
021564d4: mov    rcx, rsi
021564d7: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021564dc: test   al, al
021564de: je     0x182156665
021564e4: mov    rcx, qword ptr [rip + 0x1b3de55]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021564eb: mov    rbx, qword ptr [rdi + 0x18]
021564ef: cmp    dword ptr [rcx + 0xe0], 0
021564f6: jne    0x1821564fd
021564f8: call   0x182f60cf0
021564fd: xor    r9d, r9d
02156500: mov    r8b, 6
02156503: mov    rdx, r14
02156506: mov    rcx, rbx
02156509: call   0x182155050                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʵʽʳʷʿʾʴʻʸʿʾ
0215650e: inc    r15d
02156511: jmp    0x182155f93
02156516: cmp    eax, 0x178e3a0e
0215651b: jne    0x18215658b
0215651d: mov    rdx, qword ptr [rip + 0x1badaac]         ; [0x3d03fd0] str:'part guitar coop ghl'
02156524: xor    r8d, r8d
02156527: mov    rcx, rbx
0215652a: call   0x181981570                              ; System.String$$op_Equality
0215652f: test   al, al
02156531: je     0x182156665
02156537: test   rsi, rsi
0215653a: je     0x182156715
02156540: mov    r8, qword ptr [rip + 0x1b885c9]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
02156547: mov    dl, 0xb
02156549: mov    rcx, rsi
0215654c: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
02156551: test   al, al
02156553: je     0x182156665
02156559: mov    rcx, qword ptr [rip + 0x1b3dde0]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156560: mov    rbx, qword ptr [rdi + 0x18]
02156564: cmp    dword ptr [rcx + 0xe0], 0
0215656b: jne    0x182156572
0215656d: call   0x182f60cf0
02156572: xor    r9d, r9d
02156575: mov    r8b, 0xb
02156578: mov    rdx, r14
0215657b: mov    rcx, rbx
0215657e: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
02156583: inc    r15d
02156586: jmp    0x182155f93
0215658b: cmp    eax, 0x22c25968
02156590: jne    0x182156600
02156592: mov    rdx, qword ptr [rip + 0x1badbaf]         ; [0x3d04148] str:'part keys'
02156599: xor    r8d, r8d
0215659c: mov    rcx, rbx
0215659f: call   0x181981570                              ; System.String$$op_Equality
021565a4: test   al, al
021565a6: je     0x182156665
021565ac: test   rsi, rsi
021565af: je     0x182156715
021565b5: mov    r8, qword ptr [rip + 0x1b88554]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
021565bc: mov    dl, 7
021565be: mov    rcx, rsi
021565c1: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
021565c6: test   al, al
021565c8: je     0x182156665
021565ce: mov    rcx, qword ptr [rip + 0x1b3dd6b]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021565d5: mov    rbx, qword ptr [rdi + 0x18]
021565d9: cmp    dword ptr [rcx + 0xe0], 0
021565e0: jne    0x1821565e7
021565e2: call   0x182f60cf0
021565e7: xor    r9d, r9d
021565ea: mov    r8b, 7
021565ed: mov    rdx, r14
021565f0: mov    rcx, rbx
021565f3: call   0x182156990                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʺʴʸʶʼʺˀʶʹʶʾ
021565f8: inc    r15d
021565fb: jmp    0x182155f93
02156600: cmp    eax, 0x39c8f25a
02156605: jne    0x182156665
02156607: mov    rdx, qword ptr [rip + 0x1bade2a]         ; [0x3d04438] str:'part vocals'
0215660e: xor    r8d, r8d
02156611: mov    rcx, rbx
02156614: call   0x181981570                              ; System.String$$op_Equality
02156619: test   al, al
0215661b: je     0x182156665
0215661d: test   rsi, rsi
02156620: je     0x182156715
02156626: mov    r8, qword ptr [rip + 0x1b884e3]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
0215662d: mov    dl, 0xd
0215662f: mov    rcx, rsi
02156632: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
02156637: test   al, al
02156639: je     0x182156665
0215663b: mov    rcx, qword ptr [rip + 0x1b3dcfe]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156642: mov    rbx, qword ptr [rdi + 0x18]
02156646: cmp    dword ptr [rcx + 0xe0], 0
0215664d: jne    0x182156654
0215664f: call   0x182f60cf0
02156654: xor    r9d, r9d
02156657: mov    r8b, 0xd
0215665a: mov    rdx, r14
0215665d: mov    rcx, rbx
02156660: call   0x1821579b0                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʿʴʹʻʿʶʻʵˀʷʴ
02156665: inc    r15d
02156668: jmp    0x182155f93
0215666d: mov    rcx, qword ptr [rip + 0x1b3dccc]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156674: cmp    dword ptr [rcx + 0xe0], 0
0215667b: jne    0x182156682
0215667d: call   0x182f60cf0
02156682: cmp    byte ptr [rip + 0x1dc300e], 0            ; [0x3f19697] (bss)
02156689: jne    0x1821566aa
0215668b: lea    rcx, [rip + 0x1b762f6]                   ; [0x3ccc988] metamethod:Method$System.Threading.ThreadLocal<bool>.get_Value()
02156692: call   0x182f609b0
02156697: lea    rcx, [rip + 0x1b3dca2]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215669e: call   0x182f609b0
021566a3: mov    byte ptr [rip + 0x1dc2fed], 1            ; [0x3f19697] (bss)
021566aa: mov    rcx, qword ptr [rip + 0x1b3dc8f]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021566b1: cmp    dword ptr [rcx + 0xe0], 0
021566b8: jne    0x1821566bf
021566ba: call   0x182f60cf0
021566bf: mov    rax, qword ptr [rip + 0x1b3dc7a]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021566c6: mov    rdx, qword ptr [rax + 0xb8]
021566cd: mov    rcx, qword ptr [rdx]
021566d0: test   rcx, rcx
021566d3: je     0x182156715
021566d5: mov    rdx, qword ptr [rip + 0x1b762ac]         ; [0x3ccc988] metamethod:Method$System.Threading.ThreadLocal<bool>.get_Value()
021566dc: call   0x180b0b810                              ; System.Threading.ThreadLocal<bool>$$get_Value
021566e1: test   al, al
021566e3: jne    0x1821566ef
021566e5: xor    edx, edx
021566e7: mov    rcx, r13
021566ea: call   0x18212d820                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʽʳʶʲʾʵʴʻʽʻʺ
021566ef: mov    r15, qword ptr [rsp + 0x20]
021566f4: mov    r13, qword ptr [rsp + 0x28]
021566f9: mov    r12, qword ptr [rsp + 0x30]
021566fe: mov    rdi, qword ptr [rsp + 0x60]
02156703: mov    rsi, qword ptr [rsp + 0x58]
02156708: mov    rbp, qword ptr [rsp + 0x50]
0215670d: add    rsp, 0x38
02156711: pop    r14
02156713: pop    rbx
02156714: ret    
02156715: call   0x182f60c50
0215671a: int3   
0215671b: int3   
