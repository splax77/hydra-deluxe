021159a0: mov    qword ptr [rsp + 8], rbx
021159a5: mov    qword ptr [rsp + 0x10], rsi
021159aa: push   rdi
021159ab: sub    rsp, 0x30
021159af: cmp    byte ptr [rip + 0x1e03b68], 0            ; [0x3f1951e] (bss)
021159b6: jne    0x182115a2b
021159b8: lea    rcx, [rip + 0x1bd20f1]                   ; [0x3ce7ab0] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>..ctor()
021159bf: call   0x182f609b0
021159c4: lea    rcx, [rip + 0x1bd02ed]                   ; [0x3ce5cb8] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>..ctor()
021159cb: call   0x182f609b0
021159d0: lea    rcx, [rip + 0x1b81109]                   ; [0x3c96ae0] meta:System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>_TypeInfo
021159d7: call   0x182f609b0
021159dc: lea    rcx, [rip + 0x1b8082d]                   ; [0x3c96210] meta:System.Collections.Concurrent.ConcurrentDictionary<string, string>_TypeInfo
021159e3: call   0x182f609b0
021159e8: lea    rcx, [rip + 0x1ba31f9]                   ; [0x3cb8be8] metamethod:Method$System.IO.File.ReadAllText()
021159ef: call   0x182f609b0
021159f4: lea    rcx, [rip + 0x1bf856d]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
021159fb: call   0x182f609b0
02115a00: lea    rcx, [rip + 0x1be9ce9]                   ; [0x3cff6f0] meta:ʲʷʽʹʺʴˀʵʽˁʻ.ʶʸʹʳʹʲʻʲʽʴˁ_TypeInfo
02115a07: call   0x182f609b0
02115a0c: lea    rcx, [rip + 0x1b9e45d]                   ; [0x3cb3e70] str:''
02115a13: call   0x182f609b0
02115a18: lea    rcx, [rip + 0x1bc4229]                   ; [0x3cd9c48] str:'tmp'
02115a1f: call   0x182f609b0
02115a24: mov    byte ptr [rip + 0x1e03af3], 1            ; [0x3f1951e] (bss)
02115a2b: mov    rcx, qword ptr [rip + 0x1b810ae]         ; [0x3c96ae0] meta:System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>_TypeInfo
02115a32: call   0x182f60c00
02115a37: mov    rdx, qword ptr [rip + 0x1bd2072]         ; [0x3ce7ab0] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>..ctor()
02115a3e: mov    rcx, rax
02115a41: mov    rbx, rax
02115a44: call   0x181319b90                              ; System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, object>$$.ctor
02115a49: mov    rcx, qword ptr [rip + 0x1bf8518]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115a50: mov    rdx, qword ptr [rcx + 0xb8]
02115a57: mov    qword ptr [rdx], rbx
02115a5a: mov    rdx, rbx
02115a5d: mov    rcx, qword ptr [rip + 0x1bf8504]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115a64: mov    rcx, qword ptr [rcx + 0xb8]
02115a6b: call   0x182f5fc00
02115a70: mov    rcx, qword ptr [rip + 0x1b80799]         ; [0x3c96210] meta:System.Collections.Concurrent.ConcurrentDictionary<string, string>_TypeInfo
02115a77: call   0x182f60c00
02115a7c: mov    rdx, qword ptr [rip + 0x1bd0235]         ; [0x3ce5cb8] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>..ctor()
02115a83: mov    rcx, rax
02115a86: mov    rbx, rax
02115a89: call   0x181319470                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$.ctor
02115a8e: mov    rcx, qword ptr [rip + 0x1bf84d3]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115a95: mov    rdx, qword ptr [rcx + 0xb8]
02115a9c: mov    qword ptr [rdx + 8], rbx
02115aa0: mov    rdx, rbx
02115aa3: mov    rcx, qword ptr [rip + 0x1bf84be]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115aaa: mov    rcx, qword ptr [rcx + 0xb8]
02115ab1: add    rcx, 8
02115ab5: call   0x182f5fc00
02115aba: mov    rcx, qword ptr [rip + 0x1b8074f]         ; [0x3c96210] meta:System.Collections.Concurrent.ConcurrentDictionary<string, string>_TypeInfo
02115ac1: call   0x182f60c00
02115ac6: mov    rdx, qword ptr [rip + 0x1bd01eb]         ; [0x3ce5cb8] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>..ctor()
02115acd: mov    rcx, rax
02115ad0: mov    rbx, rax
02115ad3: call   0x181319470                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$.ctor
02115ad8: mov    rcx, qword ptr [rip + 0x1bf8489]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115adf: mov    rdx, qword ptr [rcx + 0xb8]
02115ae6: mov    qword ptr [rdx + 0x10], rbx
02115aea: mov    rdx, rbx
02115aed: mov    rcx, qword ptr [rip + 0x1bf8474]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115af4: mov    rcx, qword ptr [rcx + 0xb8]
02115afb: add    rcx, 0x10
02115aff: call   0x182f5fc00
02115b04: mov    rcx, qword ptr [rip + 0x1be9be5]         ; [0x3cff6f0] meta:ʲʷʽʹʺʴˀʵʽˁʻ.ʶʸʹʳʹʲʻʲʽʴˁ_TypeInfo
02115b0b: call   0x182f60c00
02115b10: mov    rbx, qword ptr [rip + 0x1ba30d1]         ; [0x3cb8be8] metamethod:Method$System.IO.File.ReadAllText()
02115b17: mov    rdi, rax
02115b1a: lea    rcx, [rax + 0x20]
02115b1e: mov    rdx, qword ptr [rbx + 8]
02115b22: mov    qword ptr [rax + 0x10], rdx
02115b26: xor    edx, edx
02115b28: mov    qword ptr [rax + 0x28], rbx
02115b2c: mov    qword ptr [rax + 0x20], 0
02115b34: call   0x182f5fc00
02115b39: movzx  esi, byte ptr [rbx + 0x52]
02115b3d: mov    rcx, rbx
02115b40: mov    qword ptr [rdi + 0x40], rdi
02115b44: call   0x182f5fc30
02115b49: test   al, al
02115b4b: je     0x182115b6a
02115b4d: cmp    sil, 1
02115b51: jne    0x182115b5c
02115b53: lea    rax, [rip + 0xb2cf86]                    ; [0x2c42ae0]
02115b5a: jmp    0x182115b7a
02115b5c: mov    rax, qword ptr [rdi + 0x20]
02115b60: mov    qword ptr [rdi + 0x40], rax
02115b64: mov    rax, qword ptr [rdi + 0x10]
02115b68: jmp    0x182115b7a
02115b6a: test   sil, sil
02115b6d: jne    0x182115c2a
02115b73: lea    rax, [rip + 0xb2cef6]                    ; [0x2c42a70]
02115b7a: mov    qword ptr [rdi + 0x18], rax
02115b7e: mov    rdx, rdi
02115b81: lea    rax, [rip + 0xb300b8]                    ; [0x2c45c40]
02115b88: mov    qword ptr [rdi + 0x38], rax
02115b8c: mov    rax, qword ptr [rip + 0x1bf83d5]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115b93: mov    rcx, qword ptr [rax + 0xb8]
02115b9a: mov    qword ptr [rcx + 0x20], rdi
02115b9e: mov    rax, qword ptr [rip + 0x1bf83c3]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115ba5: mov    rcx, qword ptr [rax + 0xb8]
02115bac: add    rcx, 0x20
02115bb0: call   0x182f5fc00
02115bb5: mov    r8, qword ptr [rip + 0x1bc408c]          ; [0x3cd9c48] str:'tmp'
02115bbc: lea    rcx, [rsp + 0x20]
02115bc1: mov    rdx, qword ptr [rip + 0x1b9e2a8]         ; [0x3cb3e70] str:''
02115bc8: xorps  xmm0, xmm0
02115bcb: xor    r9d, r9d
02115bce: movups xmmword ptr [rsp + 0x20], xmm0
02115bd3: call   0x1807fced0                              ; Microsoft.Extensions.Logging.LoggerMessage.LogValues<object>$$.ctor
02115bd8: mov    rax, qword ptr [rip + 0x1bf8389]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115bdf: xor    edx, edx
02115be1: movups xmm0, xmmword ptr [rsp + 0x20]
02115be6: mov    rcx, qword ptr [rax + 0xb8]
02115bed: movups xmmword ptr [rcx + 0x30], xmm0
02115bf1: mov    rax, qword ptr [rip + 0x1bf8370]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115bf8: mov    rcx, qword ptr [rax + 0xb8]
02115bff: add    rcx, 0x30
02115c03: call   0x182f5fc00
02115c08: mov    rax, qword ptr [rip + 0x1bf8359]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115c0f: mov    rbx, qword ptr [rsp + 0x40]
02115c14: mov    rsi, qword ptr [rsp + 0x48]
02115c19: mov    rcx, qword ptr [rax + 0xb8]
02115c20: mov    byte ptr [rcx + 0x40], 0
02115c24: add    rsp, 0x30
02115c28: pop    rdi
02115c29: ret    
02115c2a: lea    rdx, [rip + 0xf4e08f]                    ; [0x3063cc0]
02115c31: xor    ecx, ecx
02115c33: call   0x182f604d0
02115c38: mov    rcx, rax
02115c3b: xor    edx, edx
02115c3d: call   0x182f60c10
02115c42: int3   
02115c43: int3   
