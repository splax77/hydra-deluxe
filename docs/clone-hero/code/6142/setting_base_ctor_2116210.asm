; function 0x2116210 ʲʽˀʲʾʿʷʴʼʴʶ$$.ctor next method starts at 0x2116370 bytes 0x160
02116210: mov    qword ptr [rsp + 8], rbx
02116215: mov    qword ptr [rsp + 0x10], rbp
0211621a: push   rsi
0211621b: push   rdi
0211621c: push   r14
0211621e: sub    rsp, 0x30
02116222: cmp    byte ptr [rip + 0x1e03251], 0            ; [0x3f1947a] (bss)
02116229: mov    esi, r9d
0211622c: mov    rdi, r8
0211622f: mov    r14, rdx
02116232: mov    rbx, rcx
02116235: jne    0x18211626e
02116237: lea    rcx, [rip + 0x1bf7d2a]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
0211623e: call   0x182f609b0
02116243: lea    rcx, [rip + 0x1beb9e6]                   ; [0x3d01c30] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʷʹʾʻʶˁʿʶʴʷʷ<int>()
0211624a: call   0x182f609b0
0211624f: lea    rcx, [rip + 0x1b7cd22]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
02116256: call   0x182f609b0
0211625b: lea    rcx, [rip + 0x1be067e]                   ; [0x3cf68e0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>..ctor()
02116262: call   0x182f609b0
02116267: mov    byte ptr [rip + 0x1e0320c], 1            ; [0x3f1947a] (bss)
0211626e: mov    rdx, qword ptr [rip + 0x1be066b]         ; [0x3cf68e0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>..ctor()
02116275: mov    rcx, rbx
02116278: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
0211627d: lea    rcx, [rbx + 0x28]
02116281: mov    qword ptr [rbx + 0x28], rdi
02116285: mov    rdx, rdi
02116288: call   0x182f5fc00
0211628d: mov    rcx, qword ptr [rip + 0x1b7cce4]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
02116294: cmp    dword ptr [rcx + 0xe0], 0
0211629b: jne    0x1821162a2
0211629d: call   0x182f60cf0
021162a2: mov    rdi, qword ptr [rip + 0x1beb987]         ; [0x3d01c30] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʷʹʾʻʶˁʿʶʴʷʷ<int>()
021162a9: mov    dword ptr [rsp + 0x68], esi
021162ad: cmp    qword ptr [rdi + 0x38], 0
021162b2: jne    0x1821162bc
021162b4: mov    rcx, rdi
021162b7: call   0x182f657d0
021162bc: mov    r8, qword ptr [rdi + 0x38]
021162c0: lea    rcx, [rsp + 0x68]
021162c5: mov    esi, dword ptr [rsp + 0x70]
021162c9: mov    edx, esi
021162cb: mov    r8, qword ptr [r8 + 0x10]
021162cf: call   0x181b4e780                              ; System.Int32$$CompareTo
021162d4: mov    ebp, dword ptr [rsp + 0x78]
021162d8: test   eax, eax
021162da: js     0x1821162fb
021162dc: mov    r8, qword ptr [rdi + 0x38]
021162e0: lea    rcx, [rsp + 0x68]
021162e5: mov    edx, ebp
021162e7: mov    r8, qword ptr [r8 + 0x10]
021162eb: call   0x181b4e780                              ; System.Int32$$CompareTo
021162f0: test   eax, eax
021162f2: mov    ecx, ebp
021162f4: cmovle ecx, dword ptr [rsp + 0x68]
021162f9: jmp    0x1821162fd
021162fb: mov    ecx, esi
021162fd: mov    dword ptr [rbx + 0x1c], ecx
02116300: mov    dword ptr [rbx + 0x10], ecx
02116303: mov    dword ptr [rbx + 0x14], esi
02116306: xor    eax, eax
02116308: test   ebp, ebp
0211630a: cmovns eax, ebp
0211630d: mov    dword ptr [rbx + 0x18], eax
02116310: mov    eax, dword ptr [rsp + 0x80]
02116317: mov    dword ptr [rbx + 0x30], eax
0211631a: mov    rcx, qword ptr [rip + 0x1bf7c47]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02116321: cmp    dword ptr [rcx + 0xe0], 0
02116328: jne    0x18211632f
0211632a: call   0x182f60cf0
0211632f: movups xmm0, xmmword ptr [r14]
02116333: xor    r8d, r8d
02116336: lea    rcx, [rsp + 0x20]
0211633b: mov    rdx, rbx
0211633e: movaps xmmword ptr [rsp + 0x20], xmm0
02116343: call   0x182114840                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʿʳʻʿʻʾʹʴʴʴʺ
02116348: mov    rax, qword ptr [rbx]
0211634b: mov    rcx, rbx
0211634e: mov    rdx, qword ptr [rax + 0x2d0]
02116355: mov    rbx, qword ptr [rsp + 0x50]
0211635a: mov    rbp, qword ptr [rsp + 0x58]
0211635f: add    rsp, 0x30
02116363: pop    r14
02116365: pop    rdi
02116366: pop    rsi
02116367: jmp    qword ptr [rax + 0x2c8]
0211636e: int3   
0211636f: int3   
