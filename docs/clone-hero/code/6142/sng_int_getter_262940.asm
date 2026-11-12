00262940: mov    qword ptr [rsp + 0x18], rbx
00262945: mov    qword ptr [rsp + 0x20], rsi
0026294a: push   rdi
0026294b: sub    rsp, 0x40
0026294f: cmp    byte ptr [rip + 0x3caac8c], 0            ; [0x3f0d5e2] (bss)
00262956: mov    ebx, r8d
00262959: mov    rsi, rdx
0026295c: mov    rdi, rcx
0026295f: jne    0x180262980
00262961: lea    rcx, [rip + 0x3a6dcc8]                   ; [0x3cd0630] meta:System.Globalization.CultureInfo_TypeInfo
00262968: call   0x182f609b0
0026296d: lea    rcx, [rip + 0x3a30184]                   ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
00262974: call   0x182f609b0
00262979: mov    byte ptr [rip + 0x3caac62], 1            ; [0x3f0d5e2] (bss)
00262980: mov    rcx, qword ptr [rdi + 0x30]
00262984: mov    qword ptr [rsp + 0x30], 0
0026298d: mov    dword ptr [rsp + 0x58], 0
00262995: mov    dword ptr [rsp + 0x50], 0
0026299d: test   rcx, rcx
002629a0: je     0x180262a82
002629a6: mov    r9, qword ptr [rip + 0x3a3014b]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
002629ad: lea    r8, [rsp + 0x30]
002629b2: mov    rdx, rsi
002629b5: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
002629ba: test   al, al
002629bc: je     0x180262a70
002629c2: mov    rcx, qword ptr [rsp + 0x30]
002629c7: test   rcx, rcx
002629ca: je     0x180262a82
002629d0: mov    edx, 0x2e
002629d5: xor    r8d, r8d
002629d8: call   0x18197f870                              ; System.String$$Contains
002629dd: test   al, al
002629df: jne    0x180262a0b
002629e1: mov    rcx, qword ptr [rsp + 0x30]
002629e6: lea    rdx, [rsp + 0x50]
002629eb: xor    r8d, r8d
002629ee: call   0x181b4f710                              ; System.Int32$$TryParse
002629f3: test   al, al
002629f5: je     0x180262a70
002629f7: mov    eax, dword ptr [rsp + 0x50]
002629fb: mov    rbx, qword ptr [rsp + 0x60]
00262a00: mov    rsi, qword ptr [rsp + 0x68]
00262a05: add    rsp, 0x40
00262a09: pop    rdi
00262a0a: ret    
00262a0b: mov    rcx, qword ptr [rip + 0x3a6dc1e]         ; [0x3cd0630] meta:System.Globalization.CultureInfo_TypeInfo
00262a12: mov    rdi, qword ptr [rsp + 0x30]
00262a17: cmp    dword ptr [rcx + 0xe0], 0
00262a1e: jne    0x180262a25
00262a20: call   0x182f60cf0
00262a25: xor    ecx, ecx
00262a27: call   0x181acf580                              ; System.Globalization.CultureInfo$$get_InvariantCulture
00262a2c: lea    r9, [rsp + 0x58]
00262a31: mov    qword ptr [rsp + 0x20], 0
00262a3a: mov    r8, rax
00262a3d: mov    edx, 0xa7
00262a42: mov    rcx, rdi
00262a45: call   0x181b70450                              ; System.Single$$TryParse
00262a4a: test   al, al
00262a4c: je     0x180262a70
00262a4e: movss  xmm0, dword ptr [rsp + 0x58]
00262a54: mulss  xmm0, dword ptr [rip + 0x2e01ea8]        ; [0x3064904] flt=1000.0 i=1148846080
00262a5c: cvttss2si eax, xmm0
00262a60: mov    rbx, qword ptr [rsp + 0x60]
00262a65: mov    rsi, qword ptr [rsp + 0x68]
00262a6a: add    rsp, 0x40
00262a6e: pop    rdi
00262a6f: ret    
00262a70: mov    rsi, qword ptr [rsp + 0x68]
00262a75: mov    eax, ebx
00262a77: mov    rbx, qword ptr [rsp + 0x60]
00262a7c: add    rsp, 0x40
00262a80: pop    rdi
00262a81: ret    
00262a82: call   0x182f60c50
00262a87: int3   
00262a88: int3   
