000d0e20: push   rbp
000d0e22: sub    rsp, 0x30
000d0e26: cmp    byte ptr [rip + 0x3e3b3b4], 0            ; [0x3f0c1e1] (bss)
000d0e2d: mov    rbp, rcx
000d0e30: jne    0x1800d0e5d
000d0e32: lea    rcx, [rip + 0x3c1ea57]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d0e39: call   0x182f609b0
000d0e3e: lea    rcx, [rip + 0x3c3e203]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d0e45: call   0x182f609b0
000d0e4a: lea    rcx, [rip + 0x3bc4167]                   ; [0x3c94fb8] meta:ʶʿʷʳʾʳʶʸʻʵʽ_TypeInfo
000d0e51: call   0x182f609b0
000d0e56: mov    byte ptr [rip + 0x3e3b384], 1            ; [0x3f0c1e1] (bss)
000d0e5d: cmp    byte ptr [rbp + 0x48], 0
000d0e61: mov    qword ptr [rsp + 0x40], rbx
000d0e66: mov    qword ptr [rsp + 0x48], rsi
000d0e6b: mov    qword ptr [rsp + 0x50], rdi
000d0e70: mov    qword ptr [rsp + 0x58], r14
000d0e75: jne    0x1800d10e9
000d0e7b: xor    edi, edi
000d0e7d: mov    r14d, edi
000d0e80: mov    esi, edi
000d0e82: mov    ebx, edi
000d0e84: nop    dword ptr [rax]
000d0e88: nop    dword ptr [rax + rax]
000d0e90: mov    rcx, qword ptr [rbp + 0x30]
000d0e94: test   rcx, rcx
000d0e97: je     0x1800d11ad
000d0e9d: cmp    ebx, dword ptr [rcx + 0x18]
000d0ea0: jae    0x1800d11b3
000d0ea6: mov    eax, ebx
000d0ea8: mov    rcx, qword ptr [rcx + rax*8 + 0x20]
000d0ead: test   rcx, rcx
000d0eb0: je     0x1800d11ad
000d0eb6: xor    edx, edx
000d0eb8: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
000d0ebd: test   rax, rax
000d0ec0: je     0x1800d11ad
000d0ec6: xor    edx, edx
000d0ec8: mov    rcx, rax
000d0ecb: call   0x18287b2e0                              ; UnityEngine.GameObject$$get_activeInHierarchy
000d0ed0: test   al, al
000d0ed2: je     0x1800d0f13
000d0ed4: mov    rcx, qword ptr [rbp + 0x30]
000d0ed8: test   rcx, rcx
000d0edb: je     0x1800d11ad
000d0ee1: cmp    ebx, dword ptr [rcx + 0x18]
000d0ee4: jae    0x1800d11b3
000d0eea: mov    eax, ebx
000d0eec: mov    rcx, qword ptr [rcx + rax*8 + 0x20]
000d0ef1: test   rcx, rcx
000d0ef4: je     0x1800d11ad
000d0efa: xor    edx, edx
000d0efc: inc    r14d
000d0eff: call   0x18032f590                              ; PlayerSelectionMenu$$ʸʳʼʹʸʽʾˀʴʼʷ
000d0f04: test   al, al
000d0f06: je     0x1800d0fb4
000d0f0c: inc    esi
000d0f0e: jmp    0x1800d0fb4
000d0f13: mov    rax, qword ptr [rip + 0x3c1e976]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d0f1a: cmp    dword ptr [rax + 0xe0], edi
000d0f20: jne    0x1800d0f31
000d0f22: mov    rcx, rax
000d0f25: call   0x182f60cf0
000d0f2a: mov    rax, qword ptr [rip + 0x3c1e95f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d0f31: mov    rax, qword ptr [rax + 0xb8]
000d0f38: mov    rcx, qword ptr [rax + 8]
000d0f3c: test   rcx, rcx
000d0f3f: je     0x1800d11ad
000d0f45: mov    rcx, qword ptr [rcx + 0x38]
000d0f49: test   rcx, rcx
000d0f4c: je     0x1800d11ad
000d0f52: cmp    ebx, dword ptr [rcx + 0x18]
000d0f55: jae    0x1800d11b3
000d0f5b: mov    eax, ebx
000d0f5d: mov    rdx, qword ptr [rcx + rax*8 + 0x20]
000d0f62: test   rdx, rdx
000d0f65: je     0x1800d11ad
000d0f6b: cmp    qword ptr [rdx + 0x10], rdi
000d0f6f: je     0x1800d0fb4
000d0f71: mov    rcx, qword ptr [rbp + 0x30]
000d0f75: test   rcx, rcx
000d0f78: je     0x1800d11ad
000d0f7e: cmp    ebx, dword ptr [rcx + 0x18]
000d0f81: jae    0x1800d11b3
000d0f87: mov    eax, ebx
000d0f89: mov    rcx, qword ptr [rcx + rax*8 + 0x20]
000d0f8e: test   rcx, rcx
000d0f91: je     0x1800d11ad
000d0f97: xor    edx, edx
000d0f99: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
000d0f9e: test   rax, rax
000d0fa1: je     0x1800d11ad
000d0fa7: xor    r8d, r8d
000d0faa: mov    dl, 1
000d0fac: mov    rcx, rax
000d0faf: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
000d0fb4: inc    ebx
000d0fb6: cmp    ebx, 4
000d0fb9: jl     0x1800d0e90
000d0fbf: cmp    r14d, esi
000d0fc2: jne    0x1800d10cf
000d0fc8: test   r14d, r14d
000d0fcb: je     0x1800d10cf
000d0fd1: test   esi, esi
000d0fd3: je     0x1800d10cf
000d0fd9: mov    rbx, qword ptr [rbp + 0x40]
000d0fdd: test   rbx, rbx
000d0fe0: je     0x1800d11ad
000d0fe6: mov    eax, edi
000d0fe8: cmp    eax, dword ptr [rbx + 0x18]
000d0feb: jge    0x1800d102b
000d0fed: cmp    edi, dword ptr [rbx + 0x18]
000d0ff0: jae    0x1800d11b3
000d0ff6: movsxd rax, edi
000d0ff9: mov    rsi, qword ptr [rbx + rax*8 + 0x20]
000d0ffe: test   rsi, rsi
000d1001: je     0x1800d11ad
000d1007: xor    edx, edx
000d1009: mov    rcx, rsi
000d100c: call   0x1801e1920                              ; System.Linq.Expressions.Interpreter.LabelInfo$$get_HasDefinitions
000d1011: test   al, al
000d1013: je     0x1800d1027
000d1015: xor    edx, edx
000d1017: mov    rcx, rsi
000d101a: call   0x1801e74c0                              ; PlayerProfileMenu$$ʾʶʵʺʴˀʳʳʳʴʸ
000d101f: test   al, al
000d1021: je     0x1800d10cf
000d1027: inc    edi
000d1029: jmp    0x1800d0fe6
000d102b: cmp    byte ptr [rip + 0x3e3b1ae], 0            ; [0x3f0c1e0] (bss)
000d1032: jne    0x1800d105f
000d1034: lea    rcx, [rip + 0x3beae1d]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d103b: call   0x182f609b0
000d1040: lea    rcx, [rip + 0x3c17061]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000d1047: call   0x182f609b0
000d104c: lea    rcx, [rip + 0x3c04da5]                   ; [0x3cd5df8] str:'Gameplay'
000d1053: call   0x182f609b0
000d1058: mov    byte ptr [rip + 0x3e3b181], 1            ; [0x3f0c1e0] (bss)
000d105f: mov    rax, qword ptr [rip + 0x3beadf2]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d1066: cmp    dword ptr [rax + 0xe0], 0
000d106d: jne    0x1800d107e
000d106f: mov    rcx, rax
000d1072: call   0x182f60cf0
000d1077: mov    rax, qword ptr [rip + 0x3beadda]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d107e: mov    rax, qword ptr [rax + 0xb8]
000d1085: mov    rcx, qword ptr [rax + 8]
000d1089: test   rcx, rcx
000d108c: je     0x1800d11ad
000d1092: xor    edx, edx
000d1094: call   0x18036b6f0                              ; BassAudioManager$$ʽˁʵʵʷˀʾʴʿʷʿ
000d1099: mov    rax, qword ptr [rip + 0x3c17008]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000d10a0: mov    rcx, qword ptr [rax + 0xb8]
000d10a7: mov    rcx, qword ptr [rcx]
000d10aa: test   rcx, rcx
000d10ad: je     0x1800d11ad
000d10b3: mov    rdx, qword ptr [rip + 0x3c04d3e]         ; [0x3cd5df8] str:'Gameplay'
000d10ba: xor    r8d, r8d
000d10bd: call   0x1802583f0                              ; FadeBehaviour$$ʻʲʿʴʺʳˁʷʶʵʾ
000d10c2: xor    r8d, r8d
000d10c5: xor    edx, edx
000d10c7: mov    rcx, rbp
000d10ca: call   0x182877d00                              ; UnityEngine.Behaviour$$set_enabled
000d10cf: mov    r14, qword ptr [rsp + 0x58]
000d10d4: mov    rdi, qword ptr [rsp + 0x50]
000d10d9: mov    rsi, qword ptr [rsp + 0x48]
000d10de: mov    rbx, qword ptr [rsp + 0x40]
000d10e3: add    rsp, 0x30
000d10e7: pop    rbp
000d10e8: ret    
000d10e9: mov    rax, qword ptr [rip + 0x3c3df58]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d10f0: mov    rcx, qword ptr [rax + 0xb8]
000d10f7: mov    rcx, qword ptr [rcx]
000d10fa: test   rcx, rcx
000d10fd: je     0x1800d11ad
000d1103: xor    edx, edx
000d1105: call   0x18008f9e0                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʴˀʺʷʲʹʼʽʽʴʶ
000d110a: test   al, al
000d110c: je     0x1800d10cf
000d110e: mov    rax, qword ptr [rip + 0x3c1e77b]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d1115: cmp    dword ptr [rax + 0xe0], 0
000d111c: jne    0x1800d112d
000d111e: mov    rcx, rax
000d1121: call   0x182f60cf0
000d1126: mov    rax, qword ptr [rip + 0x3c1e763]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d112d: mov    rax, qword ptr [rax + 0xb8]
000d1134: mov    rax, qword ptr [rax + 8]
000d1138: test   rax, rax
000d113b: je     0x1800d11ad
000d113d: mov    rax, qword ptr [rax + 0x40]
000d1141: test   rax, rax
000d1144: je     0x1800d11ad
000d1146: mov    rcx, qword ptr [rax + 0x18]
000d114a: test   rcx, rcx
000d114d: je     0x1800d11ad
000d114f: xor    r8d, r8d
000d1152: xor    edx, edx
000d1154: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000d1159: test   al, al
000d115b: je     0x1800d10cf
000d1161: mov    rax, qword ptr [rip + 0x3c3dee0]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d1168: mov    rcx, qword ptr [rax + 0xb8]
000d116f: mov    rbx, qword ptr [rcx]
000d1172: mov    rcx, qword ptr [rip + 0x3bc3e3f]         ; [0x3c94fb8] meta:ʶʿʷʳʾʳʶʸʻʵʽ_TypeInfo
000d1179: call   0x182f60c00
000d117e: xor    edx, edx
000d1180: mov    rcx, rax
000d1183: mov    rsi, rax
000d1186: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
000d118b: test   rbx, rbx
000d118e: je     0x1800d11ad
000d1190: xor    edi, edi
000d1192: xor    r9d, r9d
000d1195: mov    r8b, 2
000d1198: mov    qword ptr [rsp + 0x20], rdi
000d119d: mov    rdx, rsi
000d11a0: mov    rcx, rbx
000d11a3: call   0x1800322b0                              ; ʺʸˁʼˁʶʲʼʻˀʵ$$ˀˁʲʼʽʽʸʺʾʹʺ
000d11a8: jmp    0x1800d10cf
000d11ad: call   0x182f60c50
000d11b2: int3   
000d11b3: call   0x182f60c40
000d11b8: int3   
000d11b9: int3   
