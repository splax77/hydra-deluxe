01ee9800: mov    qword ptr [rsp + 0x20], rbx
01ee9805: push   rdi
01ee9806: sub    rsp, 0x30
01ee980a: cmp    byte ptr [rip + 0x202d321], 0            ; [0x3f16b32] (bss)
01ee9811: mov    ebx, edx
01ee9813: mov    rdi, rcx
01ee9816: jne    0x181ee9843
01ee9818: lea    rcx, [rip + 0x1de7891]                   ; [0x3cd10b0] meta:Rewired.CustomController_TypeInfo
01ee981f: call   0x182f609b0
01ee9824: lea    rcx, [rip + 0x1e03d25]                   ; [0x3ced550] meta:GIVBMWsXQJWUnIConHwoyWdNcSEC_TypeInfo
01ee982b: call   0x182f609b0
01ee9830: lea    rcx, [rip + 0x1dd67f1]                   ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
01ee9837: call   0x182f609b0
01ee983c: mov    byte ptr [rip + 0x202d2ef], 1            ; [0x3f16b32] (bss)
01ee9843: mov    rcx, qword ptr [rip + 0x1dd67de]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
01ee984a: mov    dword ptr [rsp + 0x48], 0
01ee9852: cmp    dword ptr [rcx + 0xe0], 0
01ee9859: jne    0x181ee9860
01ee985b: call   0x182f60cf0
01ee9860: cmp    byte ptr [rip + 0x202c8e5], 0            ; [0x3f1614c] (bss)
01ee9867: jne    0x181ee987c
01ee9869: lea    rcx, [rip + 0x1dd67b8]                   ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
01ee9870: call   0x182f609b0
01ee9875: mov    byte ptr [rip + 0x202c8d0], 1            ; [0x3f1614c] (bss)
01ee987c: mov    rcx, qword ptr [rip + 0x1dd67a5]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
01ee9883: cmp    dword ptr [rcx + 0xe0], 0
01ee988a: jne    0x181ee9891
01ee988c: call   0x182f60cf0
01ee9891: mov    rax, qword ptr [rip + 0x1dd6790]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
01ee9898: mov    qword ptr [rsp + 0x40], rbp
01ee989d: mov    qword ptr [rsp + 0x50], rsi
01ee98a2: mov    rcx, qword ptr [rax + 0xb8]
01ee98a9: mov    rcx, qword ptr [rcx + 0x30]
01ee98ad: test   rcx, rcx
01ee98b0: je     0x181ee9b16
01ee98b6: xor    r8d, r8d
01ee98b9: mov    edx, ebx
01ee98bb: call   0x181db1430                              ; Rewired.Data.UserData$$GetCustomControllerById
01ee98c0: mov    rsi, rax
01ee98c3: test   rax, rax
01ee98c6: je     0x181ee9b14
01ee98cc: mov    ebp, dword ptr [rdi + 0xd0]
01ee98d2: lea    ecx, [rbp + 1]
01ee98d5: mov    dword ptr [rdi + 0xd0], ecx
01ee98db: cmp    ecx, 0x7fffffff
01ee98e1: jl     0x181ee98ed
01ee98e3: mov    dword ptr [rdi + 0xd0], 0
01ee98ed: mov    rcx, qword ptr [rip + 0x1e03c5c]         ; [0x3ced550] meta:GIVBMWsXQJWUnIConHwoyWdNcSEC_TypeInfo
01ee98f4: call   0x182f60c00
01ee98f9: xor    edx, edx
01ee98fb: mov    rcx, rax
01ee98fe: mov    rbx, rax
01ee9901: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
01ee9906: test   rbx, rbx
01ee9909: je     0x181ee9b16
01ee990f: mov    dword ptr [rbx + 0x28], 0x64
01ee9916: lea    rcx, [rbx + 0x30]
01ee991a: mov    rdx, qword ptr [rsi + 0x18]
01ee991e: mov    qword ptr [rbx + 0x30], rdx
01ee9922: call   0x182f5fc00
01ee9927: mov    rdx, qword ptr [rsi + 0x10]
01ee992b: lea    rcx, [rbx + 0x38]
01ee992f: mov    qword ptr [rbx + 0x38], rdx
01ee9933: call   0x182f5fc00
01ee9938: xor    edx, edx
01ee993a: mov    rcx, rsi
01ee993d: call   0x181da6b40                              ; Rewired.Data.CustomController_Editor$$get_axisCount
01ee9942: xor    edx, edx
01ee9944: mov    dword ptr [rbx + 0x48], eax
01ee9947: mov    rcx, rsi
01ee994a: call   0x181da6b90                              ; Rewired.Data.CustomController_Editor$$get_buttonCount
01ee994f: mov    dword ptr [rbx + 0x4c], eax
01ee9952: lea    rcx, [rsp + 0x20]
01ee9957: mov    dword ptr [rbx + 0x10], ebp
01ee995a: xor    r8d, r8d
01ee995d: mov    eax, dword ptr [rsi + 0x20]
01ee9960: mov    rdx, rsi
01ee9963: mov    dword ptr [rbx + 0x14], eax
01ee9966: call   0x181da6be0                              ; Rewired.Data.CustomController_Editor$$get_typeGuid
01ee996b: xor    edx, edx
01ee996d: lea    rcx, [rsp + 0x48]
01ee9972: movups xmm0, xmmword ptr [rax]
01ee9975: movups xmmword ptr [rbx + 0x18], xmm0
01ee9979: mov    eax, dword ptr [rsi + 0x20]
01ee997c: mov    dword ptr [rsp + 0x48], eax
01ee9980: call   0x181b4f130                              ; System.Int32$$ToString
01ee9985: lea    rcx, [rbx + 0x40]
01ee9989: mov    qword ptr [rbx + 0x40], rax
01ee998d: mov    rdx, rax
01ee9990: call   0x182f5fc00
01ee9995: xor    edx, edx
01ee9997: mov    rcx, rsi
01ee999a: call   0x181da39c0                              ; Rewired.Data.CustomController_Editor$$CreateGameHardwareMap
01ee999f: lea    rcx, [rbx + 0x50]
01ee99a3: mov    qword ptr [rbx + 0x50], rax
01ee99a7: mov    rdx, rax
01ee99aa: call   0x182f5fc00
01ee99af: mov    rcx, qword ptr [rip + 0x1de76fa]         ; [0x3cd10b0] meta:Rewired.CustomController_TypeInfo
01ee99b6: call   0x182f60c00
01ee99bb: xor    r8d, r8d
01ee99be: mov    rdx, rbx
01ee99c1: mov    rcx, rax
01ee99c4: mov    rsi, rax
01ee99c7: call   0x181d5e790                              ; Rewired.CustomController$$.ctor
01ee99cc: cmp    byte ptr [rip + 0x202d18e], 0            ; [0x3f16b61] (bss)
01ee99d3: jne    0x181ee9a0c
01ee99d5: lea    rcx, [rip + 0x1e21b04]                   ; [0x3d0b4e0] metamethod:Method$System.Collections.Generic.List<CustomController>.Add()
01ee99dc: call   0x182f609b0
01ee99e1: lea    rcx, [rip + 0x1e1dec8]                   ; [0x3d078b0] metamethod:Method$System.Collections.Generic.List<Controller>.Add()
01ee99e8: call   0x182f609b0
01ee99ed: lea    rcx, [rip + 0x1e21c64]                   ; [0x3d0b658] metamethod:Method$System.Collections.Generic.List<CustomController>.get_Count()
01ee99f4: call   0x182f609b0
01ee99f9: lea    rcx, [rip + 0x1e21d10]                   ; [0x3d0b710] metamethod:Method$System.Collections.Generic.List<CustomController>.get_Item()
01ee9a00: call   0x182f609b0
01ee9a05: mov    byte ptr [rip + 0x202d155], 1            ; [0x3f16b61] (bss)
01ee9a0c: test   rsi, rsi
01ee9a0f: je     0x181ee9afc
01ee9a15: mov    rcx, qword ptr [rdi + 0x20]
01ee9a19: xor    ebx, ebx
01ee9a1b: xor    edx, edx
01ee9a1d: test   rcx, rcx
01ee9a20: je     0x181ee9b16
01ee9a26: mov    rax, qword ptr [rdi + 0x20]
01ee9a2a: cmp    edx, dword ptr [rcx + 0x18]
01ee9a2d: jge    0x181ee9a65
01ee9a2f: test   rax, rax
01ee9a32: je     0x181ee9b16
01ee9a38: mov    r8, qword ptr [rip + 0x1e21cd1]          ; [0x3d0b710] metamethod:Method$System.Collections.Generic.List<CustomController>.get_Item()
01ee9a3f: mov    edx, ebx
01ee9a41: mov    rcx, rax
01ee9a44: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
01ee9a49: cmp    rax, rsi
01ee9a4c: je     0x181ee9afc
01ee9a52: mov    rcx, qword ptr [rdi + 0x20]
01ee9a56: inc    ebx
01ee9a58: mov    edx, ebx
01ee9a5a: test   rcx, rcx
01ee9a5d: je     0x181ee9b16
01ee9a63: jmp    0x181ee9a26
01ee9a65: test   rax, rax
01ee9a68: je     0x181ee9b16
01ee9a6e: mov    r9, qword ptr [rip + 0x1e21a6b]          ; [0x3d0b4e0] metamethod:Method$System.Collections.Generic.List<CustomController>.Add()
01ee9a75: inc    dword ptr [rax + 0x1c]
01ee9a78: mov    rdx, qword ptr [rax + 0x10]
01ee9a7c: test   rdx, rdx
01ee9a7f: je     0x181ee9b16
01ee9a85: movsxd r8, dword ptr [rax + 0x18]
01ee9a89: cmp    r8d, dword ptr [rdx + 0x18]
01ee9a8d: jb     0x181ee9aab
01ee9a8f: mov    rcx, qword ptr [r9 + 0x20]
01ee9a93: mov    rdx, rsi
01ee9a96: mov    r8, qword ptr [rcx + 0xc0]
01ee9a9d: mov    rcx, rax
01ee9aa0: mov    r8, qword ptr [r8 + 0x70]
01ee9aa4: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
01ee9aa9: jmp    0x181ee9acd
01ee9aab: lea    ecx, [r8 + 1]
01ee9aaf: mov    dword ptr [rax + 0x18], ecx
01ee9ab2: cmp    r8d, dword ptr [rdx + 0x18]
01ee9ab6: jae    0x181ee9b1c
01ee9ab8: mov    qword ptr [rdx + r8*8 + 0x20], rsi
01ee9abd: add    rdx, 0x20
01ee9ac1: lea    rcx, [rdx + r8*8]
01ee9ac5: mov    rdx, rsi
01ee9ac8: call   0x182f5fc00
01ee9acd: mov    rcx, qword ptr [rdi + 0x28]
01ee9ad1: test   rcx, rcx
01ee9ad4: je     0x181ee9b16
01ee9ad6: mov    r8, qword ptr [rip + 0x1e1ddd3]          ; [0x3d078b0] metamethod:Method$System.Collections.Generic.List<Controller>.Add()
01ee9add: mov    rdx, rsi
01ee9ae0: call   0x182c425f0
01ee9ae5: mov    rcx, qword ptr [rdi + 0xa0]
01ee9aec: test   rcx, rcx
01ee9aef: je     0x181ee9b16
01ee9af1: xor    r8d, r8d
01ee9af4: mov    rdx, rsi
01ee9af7: call   0x181d496c0                              ; qsdBhRHZOfeRWePeZLKTjSjCmcvV$$CjniIjbuHWGWWlHZeoWhtgdSSoHS
01ee9afc: mov    rax, rsi
01ee9aff: mov    rsi, qword ptr [rsp + 0x50]
01ee9b04: mov    rbp, qword ptr [rsp + 0x40]
01ee9b09: mov    rbx, qword ptr [rsp + 0x58]
01ee9b0e: add    rsp, 0x30
01ee9b12: pop    rdi
01ee9b13: ret    
01ee9b14: jmp    0x181ee9aff
01ee9b16: call   0x182f60c50
01ee9b1b: int3   
01ee9b1c: call   0x182f60c40
01ee9b21: int3   
01ee9b22: int3   
