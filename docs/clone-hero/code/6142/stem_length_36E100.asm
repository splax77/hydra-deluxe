0036e100: mov    qword ptr [rsp + 0x10], rbx
0036e105: push   rdi
0036e106: sub    rsp, 0x30
0036e10a: cmp    byte ptr [rip + 0x3b9ffc3], 0            ; [0x3f0e0d4] (bss)
0036e111: mov    ebx, ecx
0036e113: jne    0x18036e158
0036e115: lea    rcx, [rip + 0x396a234]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0036e11c: call   0x182f609b0
0036e121: lea    rcx, [rip + 0x3927a50]                   ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
0036e128: call   0x182f609b0
0036e12d: lea    rcx, [rip + 0x392a654]                   ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
0036e134: call   0x182f609b0
0036e139: lea    rcx, [rip + 0x3952f40]                   ; [0x3cc1080] str:'Failed to convert length {0}'
0036e140: call   0x182f609b0
0036e145: lea    rcx, [rip + 0x3954974]                   ; [0x3cc2ac0] str:'Failed to get audio length {0}'
0036e14c: call   0x182f609b0
0036e151: mov    byte ptr [rip + 0x3b9ff7c], 1            ; [0x3f0e0d4] (bss)
0036e158: mov    rcx, qword ptr [rip + 0x3927a19]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
0036e15f: cmp    dword ptr [rcx + 0xe0], 0
0036e166: jne    0x18036e16d
0036e168: call   0x182f60cf0
0036e16d: xor    r8d, r8d
0036e170: xor    edx, edx
0036e172: mov    ecx, ebx
0036e174: call   0x18024a6e0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ʺʵˀʳʾʽʻˀʿʳʵ
0036e179: mov    rdi, rax
0036e17c: mov    rcx, qword ptr [rip + 0x39279f5]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
0036e183: cmp    rax, -1
0036e187: je     0x18036e22e
0036e18d: cmp    dword ptr [rcx + 0xe0], 0
0036e194: movaps xmmword ptr [rsp + 0x20], xmm6
0036e199: jne    0x18036e1a0
0036e19b: call   0x182f60cf0
0036e1a0: xor    r8d, r8d
0036e1a3: mov    rdx, rdi
0036e1a6: mov    ecx, ebx
0036e1a8: call   0x18024dc30                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˀʵʺʼʾʷˀˀʿˁʿ
0036e1ad: xorps  xmm6, xmm6
0036e1b0: comisd xmm6, xmm0
0036e1b4: jbe    0x18036e21e
0036e1b6: mov    rcx, qword ptr [rip + 0x39279bb]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
0036e1bd: cmp    dword ptr [rcx + 0xe0], 0
0036e1c4: jne    0x18036e1cb
0036e1c6: call   0x182f60cf0
0036e1cb: xor    ecx, ecx
0036e1cd: call   0x18024edf0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˁʼʿʶʳˀʼʾʳˁʲ
0036e1d2: mov    rcx, qword ptr [rip + 0x392a5af]         ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
0036e1d9: lea    rdx, [rsp + 0x40]
0036e1de: mov    dword ptr [rsp + 0x40], eax
0036e1e2: call   0x182f5fbe0
0036e1e7: mov    rcx, qword ptr [rip + 0x3952e92]         ; [0x3cc1080] str:'Failed to convert length {0}'
0036e1ee: xor    r8d, r8d
0036e1f1: mov    rdx, rax
0036e1f4: call   0x181982590                              ; System.String$$Format
0036e1f9: mov    rcx, qword ptr [rip + 0x396a150]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0036e200: mov    rbx, rax
0036e203: cmp    dword ptr [rcx + 0xe0], 0
0036e20a: jne    0x18036e211
0036e20c: call   0x182f60cf0
0036e211: xor    edx, edx
0036e213: mov    rcx, rbx
0036e216: call   0x18285fb70                              ; UnityEngine.Debug$$LogWarning
0036e21b: xorps  xmm0, xmm0
0036e21e: movaps xmm6, xmmword ptr [rsp + 0x20]
0036e223: mov    rbx, qword ptr [rsp + 0x48]
0036e228: add    rsp, 0x30
0036e22c: pop    rdi
0036e22d: ret    
0036e22e: cmp    dword ptr [rcx + 0xe0], 0
0036e235: jne    0x18036e23c
0036e237: call   0x182f60cf0
0036e23c: xor    ecx, ecx
0036e23e: call   0x18024edf0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˁʼʿʶʳˀʼʾʳˁʲ
0036e243: mov    rcx, qword ptr [rip + 0x392a53e]         ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
0036e24a: lea    rdx, [rsp + 0x40]
0036e24f: mov    dword ptr [rsp + 0x40], eax
0036e253: call   0x182f5fbe0
0036e258: mov    rcx, qword ptr [rip + 0x3954861]         ; [0x3cc2ac0] str:'Failed to get audio length {0}'
0036e25f: xor    r8d, r8d
0036e262: mov    rdx, rax
0036e265: call   0x181982590                              ; System.String$$Format
0036e26a: mov    rcx, qword ptr [rip + 0x396a0df]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0036e271: mov    rbx, rax
0036e274: cmp    dword ptr [rcx + 0xe0], 0
0036e27b: jne    0x18036e282
0036e27d: call   0x182f60cf0
0036e282: xor    edx, edx
0036e284: mov    rcx, rbx
0036e287: call   0x18285fb70                              ; UnityEngine.Debug$$LogWarning
0036e28c: mov    rbx, qword ptr [rsp + 0x48]
0036e291: xorps  xmm0, xmm0
0036e294: add    rsp, 0x30
0036e298: pop    rdi
0036e299: ret    
0036e29a: int3   
