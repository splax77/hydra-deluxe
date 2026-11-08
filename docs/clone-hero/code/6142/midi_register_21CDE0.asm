0021cde0: mov    qword ptr [rsp + 0x18], rbx
0021cde5: mov    qword ptr [rsp + 8], rcx
0021cdea: push   rsi
0021cdeb: push   rdi
0021cdec: push   r12
0021cdee: push   r14
0021cdf0: push   r15
0021cdf2: sub    rsp, 0x40
0021cdf6: mov    ebx, r8d
0021cdf9: mov    rsi, rdx
0021cdfc: mov    rdi, rcx
0021cdff: cmp    byte ptr [rip + 0x3cf005d], 0            ; [0x3f0ce63] (bss)
0021ce06: jne    0x18021ce97
0021ce0c: lea    rcx, [rip + 0x3a95d5d]                   ; [0x3cb2b70] meta:System.Action_TypeInfo
0021ce13: call   0x182f609b0
0021ce18: lea    rcx, [rip + 0x3ac6ef1]                   ; [0x3ce3d10] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.Add()
0021ce1f: call   0x182f609b0
0021ce24: lea    rcx, [rip + 0x3ac65e5]                   ; [0x3ce3410] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>..ctor()
0021ce2b: call   0x182f609b0
0021ce30: lea    rcx, [rip + 0x3ac7059]                   ; [0x3ce3e90] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Count()
0021ce37: call   0x182f609b0
0021ce3c: lea    rcx, [rip + 0x3a9bd9d]                   ; [0x3cb8be0] metamethod:Method$System.Collections.Generic.List<string>.get_Item()
0021ce43: call   0x182f609b0
0021ce48: lea    rcx, [rip + 0x3adb569]                   ; [0x3cf83b8] meta:System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>_TypeInfo
0021ce4f: call   0x182f609b0
0021ce54: lea    rcx, [rip + 0x3ae32e5]                   ; [0x3d00140] metamethod:Method$MIDIInputManager.ʸʲˀʸʲʻˀʶʲʲʲ()
0021ce5b: call   0x182f609b0
0021ce60: lea    rcx, [rip + 0x3aa31c1]                   ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
0021ce67: call   0x182f609b0
0021ce6c: lea    rcx, [rip + 0x3aac8ad]                   ; [0x3cc9720] meta:MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ_TypeInfo
0021ce73: call   0x182f609b0
0021ce78: lea    rcx, [rip + 0x3aa8a89]                   ; [0x3cc5908] str:'RtMidi'
0021ce7f: call   0x182f609b0
0021ce84: lea    rcx, [rip + 0x3a96fe5]                   ; [0x3cb3e70] str:''
0021ce8b: call   0x182f609b0
0021ce90: mov    byte ptr [rip + 0x3ceffcc], 1            ; [0x3f0ce63] (bss)
0021ce97: test   rsi, rsi
0021ce9a: je     0x18021d25b
0021cea0: mov    rax, qword ptr [rsi + 0x10]
0021cea4: test   rax, rax
0021cea7: je     0x18021d25b
0021cead: cmp    byte ptr [rax + 0xb1], 0
0021ceb4: jne    0x18021d216
0021ceba: cmp    byte ptr [rip + 0x3ceffba], 0            ; [0x3f0ce7b] (bss)
0021cec1: jne    0x18021ced6
0021cec3: lea    rcx, [rip + 0x3ad9ad6]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0021ceca: call   0x182f609b0
0021cecf: mov    byte ptr [rip + 0x3ceffa5], 1            ; [0x3f0ce7b] (bss)
0021ced6: cmp    qword ptr [rsi + 0x10], 0
0021cedb: je     0x18021d216
0021cee1: mov    rax, qword ptr [rsi + 0x10]
0021cee5: mov    rcx, qword ptr [rax + 0x60]
0021cee9: test   rcx, rcx
0021ceec: je     0x18021d25b
0021cef2: mov    eax, dword ptr [rcx + 0x10]
0021cef5: cmp    al, 2
0021cef7: jne    0x18021cefd
0021cef9: mov    al, 1
0021cefb: jmp    0x18021cf02
0021cefd: cmp    al, 3
0021ceff: sete   al
0021cf02: test   al, al
0021cf04: je     0x18021d216
0021cf0a: xor    r8d, r8d
0021cf0d: mov    rdx, rsi
0021cf10: mov    rcx, rdi
0021cf13: call   0x18021f320                              ; MIDIInputManager$$ʸˀʵʾʻʳʲʸʶʸʿ
0021cf18: cmp    ebx, -1
0021cf1b: je     0x18021d22b
0021cf21: mov    rcx, qword ptr [rdi + 0x20]
0021cf25: test   rcx, rcx
0021cf28: je     0x18021d299
0021cf2e: xor    edx, edx
0021cf30: call   0x18207b3b0                              ; RtMidi.MidiBase$$_GetPortCount
0021cf35: dec    eax
0021cf37: cmp    ebx, eax
0021cf39: jg     0x18021d164
0021cf3f: xor    edx, edx
0021cf41: mov    rcx, rdi
0021cf44: call   0x18021f120                              ; MIDIInputManager$$ʸʿʻʴʴʼʵʴʳˀʵ
0021cf49: test   rax, rax
0021cf4c: je     0x18021d294
0021cf52: mov    r8, qword ptr [rip + 0x3a9bc87]          ; [0x3cb8be0] metamethod:Method$System.Collections.Generic.List<string>.get_Item()
0021cf59: mov    edx, ebx
0021cf5b: mov    rcx, rax
0021cf5e: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0021cf63: mov    r15, rax
0021cf66: xor    r8d, r8d
0021cf69: mov    rdx, rax
0021cf6c: mov    rcx, rdi
0021cf6f: call   0x18021df80                              ; MIDIInputManager$$ʵʾʸʿʺʺˀʿʸʸʹ
0021cf74: test   rax, rax
0021cf77: jne    0x18021d15f
0021cf7d: xor    ecx, ecx
0021cf7f: call   0x18207bac0                              ; RtMidi.MidiIn$$Create
0021cf84: mov    r14, rax
0021cf87: test   rax, rax
0021cf8a: je     0x18021d28f
0021cf90: xor    r9d, r9d
0021cf93: mov    r8, qword ptr [rip + 0x3aa896e]          ; [0x3cc5908] str:'RtMidi'
0021cf9a: mov    edx, ebx
0021cf9c: mov    rcx, rax
0021cf9f: call   0x18207ad90                              ; RtMidi.MidiBase$$OpenPort
0021cfa4: mov    rcx, qword ptr [rip + 0x3aa307d]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
0021cfab: cmp    dword ptr [rcx + 0xe0], 0
0021cfb2: jne    0x18021cfb9
0021cfb4: call   0x182f60cf0
0021cfb9: xor    ecx, ecx
0021cfbb: call   0x181f73330                              ; Rewired.ReInput$$get_controllers
0021cfc0: test   rax, rax
0021cfc3: je     0x18021d28a
0021cfc9: xor    r8d, r8d
0021cfcc: mov    edx, 1
0021cfd1: mov    rcx, rax
0021cfd4: call   0x181d6fa80                              ; Rewired.ReInput.ControllerHelper$$CreateCustomController
0021cfd9: mov    r12, rax
0021cfdc: mov    rcx, qword ptr [rsi + 0x18]
0021cfe0: test   rcx, rcx
0021cfe3: je     0x18021d285
0021cfe9: mov    rcx, qword ptr [rcx + 0x50]
0021cfed: test   rcx, rcx
0021cff0: je     0x18021d280
0021cff6: xor    r9d, r9d
0021cff9: xor    r8d, r8d
0021cffc: mov    rdx, rax
0021cfff: call   0x181f2d2b0                              ; Rewired.Player.ControllerHelper$$AddController
0021d004: mov    rcx, qword ptr [rip + 0x3aac715]         ; [0x3cc9720] meta:MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ_TypeInfo
0021d00b: call   0x182f60c00
0021d010: mov    rbx, rax
0021d013: xor    edx, edx
0021d015: mov    rcx, rax
0021d018: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0021d01d: test   rbx, rbx
0021d020: je     0x18021d27b
0021d026: mov    qword ptr [rbx + 0x18], r14
0021d02a: lea    rcx, [rbx + 0x18]
0021d02e: mov    rdx, r14
0021d031: call   0x182f5fc00
0021d036: mov    qword ptr [rbx + 0x40], r15
0021d03a: lea    rcx, [rbx + 0x40]
0021d03e: mov    rdx, r15
0021d041: call   0x182f5fc00
0021d046: mov    qword ptr [rbx + 0x10], rsi
0021d04a: lea    rcx, [rbx + 0x10]
0021d04e: mov    rdx, rsi
0021d051: call   0x182f5fc00
0021d056: mov    qword ptr [rbx + 0x30], r12
0021d05a: lea    rcx, [rbx + 0x30]
0021d05e: mov    rdx, r12
0021d061: call   0x182f5fc00
0021d066: mov    rcx, qword ptr [rip + 0x3adb34b]         ; [0x3cf83b8] meta:System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>_TypeInfo
0021d06d: call   0x182f60c00
0021d072: mov    r14, rax
0021d075: mov    rdx, qword ptr [rip + 0x3ac6394]         ; [0x3ce3410] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>..ctor()
0021d07c: mov    rcx, rax
0021d07f: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
0021d084: mov    qword ptr [rbx + 0x38], r14
0021d088: lea    rcx, [rbx + 0x38]
0021d08c: mov    rdx, r14
0021d08f: call   0x182f5fc00
0021d094: mov    byte ptr [rbx + 0x48], 1
0021d098: xor    r9d, r9d
0021d09b: mov    r8, r15
0021d09e: mov    rdx, rdi
0021d0a1: lea    rcx, [rsp + 0x30]
0021d0a6: call   0x18021fc70                              ; MIDIInputManager$$ʺˀʹˀʽʼʹʵʺʶʻ
0021d0ab: movups xmm0, xmmword ptr [rax]
0021d0ae: movups xmmword ptr [rbx + 0x20], xmm0
0021d0b2: lea    rcx, [rbx + 0x20]
0021d0b6: xor    edx, edx
0021d0b8: call   0x182f5fc00
0021d0bd: mov    rcx, qword ptr [rdi + 0x38]
0021d0c1: test   rcx, rcx
0021d0c4: je     0x18021d276
0021d0ca: mov    r9, qword ptr [rip + 0x3ac6c3f]          ; [0x3ce3d10] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.Add()
0021d0d1: inc    dword ptr [rcx + 0x1c]
0021d0d4: mov    rdx, qword ptr [rcx + 0x10]
0021d0d8: test   rdx, rdx
0021d0db: je     0x18021d271
0021d0e1: movsxd r8, dword ptr [rcx + 0x18]
0021d0e5: cmp    r8d, dword ptr [rdx + 0x18]
0021d0e9: jb     0x18021d104
0021d0eb: mov    rax, qword ptr [r9 + 0x20]
0021d0ef: mov    r8, qword ptr [rax + 0xc0]
0021d0f6: mov    r8, qword ptr [r8 + 0x70]
0021d0fa: mov    rdx, rbx
0021d0fd: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0021d102: jmp    0x18021d12a
0021d104: lea    eax, [r8 + 1]
0021d108: mov    dword ptr [rcx + 0x18], eax
0021d10b: cmp    r8d, dword ptr [rdx + 0x18]
0021d10f: jae    0x18021d261
0021d115: mov    qword ptr [rdx + r8*8 + 0x20], rbx
0021d11a: add    rdx, 0x20
0021d11e: lea    rcx, [rdx + r8*8]
0021d122: mov    rdx, rbx
0021d125: call   0x182f5fc00
0021d12a: mov    rax, qword ptr [rsi + 0x10]
0021d12e: test   rax, rax
0021d131: je     0x18021d26c
0021d137: mov    rcx, qword ptr [rax + 0xd0]
0021d13e: test   rcx, rcx
0021d141: je     0x18021d267
0021d147: xor    r8d, r8d
0021d14a: mov    rdx, r15
0021d14d: call   0x18212b040                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʸʾʵʽʳʾʾʻʷʲʽ
0021d152: xor    edx, edx
0021d154: mov    rcx, rdi
0021d157: call   0x18021e270                              ; MIDIInputManager$$ʷʽʺʷˁʲʽʴʽʲˁ
0021d15c: nop    
0021d15d: jmp    0x18021d1b6
0021d15f: jmp    0x18021d216
0021d164: jmp    0x18021d216
0021d169: mov    rcx, qword ptr [rsp + 0x78]
0021d16e: test   rcx, rcx
0021d171: je     0x18021d25b
0021d177: mov    rax, qword ptr [rcx]
0021d17a: mov    rdx, qword ptr [rax + 0x190]
0021d181: call   qword ptr [rax + 0x188]
0021d187: mov    rbx, rax
0021d18a: lea    rcx, [rip + 0x3abb1bf]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0021d191: call   0x182f609d0
0021d196: cmp    dword ptr [rax + 0xe0], 0
0021d19d: jne    0x18021d1a7
0021d19f: mov    rcx, rax
0021d1a2: call   0x182f60cf0
0021d1a7: xor    edx, edx
0021d1a9: mov    rcx, rbx
0021d1ac: call   0x18285f400                              ; UnityEngine.Debug$$LogError
0021d1b1: mov    rdi, qword ptr [rsp + 0x70]
0021d1b6: mov    rax, qword ptr [rdi + 0x38]
0021d1ba: test   rax, rax
0021d1bd: je     0x18021d25b
0021d1c3: cmp    dword ptr [rax + 0x18], 0
0021d1c7: jle    0x18021d216
0021d1c9: cmp    byte ptr [rdi + 0x28], 0
0021d1cd: jne    0x18021d216
0021d1cf: mov    rcx, qword ptr [rip + 0x3a9599a]         ; [0x3cb2b70] meta:System.Action_TypeInfo
0021d1d6: call   0x182f60c00
0021d1db: mov    rbx, rax
0021d1de: xor    r9d, r9d
0021d1e1: mov    r8, qword ptr [rip + 0x3ae2f58]          ; [0x3d00140] metamethod:Method$MIDIInputManager.ʸʲˀʸʲʻˀʶʲʲʲ()
0021d1e8: mov    rdx, rdi
0021d1eb: mov    rcx, rax
0021d1ee: call   0x180026bd0                              ; UnityEngine.AI.NavMesh.OnNavMeshPreUpdate$$.ctor
0021d1f3: mov    rcx, qword ptr [rip + 0x3aa2e2e]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
0021d1fa: cmp    dword ptr [rcx + 0xe0], 0
0021d201: jne    0x18021d208
0021d203: call   0x182f60cf0
0021d208: xor    edx, edx
0021d20a: mov    rcx, rbx
0021d20d: call   0x181f704d0                              ; Rewired.ReInput$$add_InputSourceUpdateEvent
0021d212: mov    byte ptr [rdi + 0x28], 1
0021d216: mov    rbx, qword ptr [rsp + 0x80]
0021d21e: add    rsp, 0x40
0021d222: pop    r15
0021d224: pop    r14
0021d226: pop    r12
0021d228: pop    rdi
0021d229: pop    rsi
0021d22a: ret    
0021d22b: mov    rax, qword ptr [rsi + 0x10]
0021d22f: test   rax, rax
0021d232: je     0x18021d25b
0021d234: mov    rcx, qword ptr [rax + 0xd0]
0021d23b: test   rcx, rcx
0021d23e: je     0x18021d25b
0021d240: xor    r8d, r8d
0021d243: mov    rdx, qword ptr [rip + 0x3a96c26]         ; [0x3cb3e70] str:''
0021d24a: call   0x18212b040                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʸʾʵʽʳʾʾʻʷʲʽ
0021d24f: xor    edx, edx
0021d251: mov    rcx, rdi
0021d254: call   0x18021e270                              ; MIDIInputManager$$ʷʽʺʷˁʲʽʴʽʲˁ
0021d259: jmp    0x18021d216
0021d25b: call   0x182f60c50
0021d260: nop    
0021d261: call   0x182f60c40
0021d266: nop    
0021d267: call   0x182f60c50
0021d26c: call   0x182f60c50
0021d271: call   0x182f60c50
0021d276: call   0x182f60c50
0021d27b: call   0x182f60c50
0021d280: call   0x182f60c50
0021d285: call   0x182f60c50
0021d28a: call   0x182f60c50
0021d28f: call   0x182f60c50
0021d294: call   0x182f60c50
0021d299: call   0x182f60c50
0021d29e: int3   
0021d29f: int3   
