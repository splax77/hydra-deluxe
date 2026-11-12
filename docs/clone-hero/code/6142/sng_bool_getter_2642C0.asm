002642c0: mov    qword ptr [rsp + 0x18], rbx
002642c5: mov    qword ptr [rsp + 0x20], rsi
002642ca: push   rdi
002642cb: sub    rsp, 0x20
002642cf: cmp    byte ptr [rip + 0x3ca9315], 0            ; [0x3f0d5eb] (bss)
002642d6: movzx  edi, r8b
002642da: mov    rsi, rdx
002642dd: mov    rbx, rcx
002642e0: jne    0x180264301
002642e2: lea    rcx, [rip + 0x3a5ad37]                   ; [0x3cbf020] meta:bool_TypeInfo
002642e9: call   0x182f609b0
002642ee: lea    rcx, [rip + 0x3a2e803]                   ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
002642f5: call   0x182f609b0
002642fa: mov    byte ptr [rip + 0x3ca92ea], 1            ; [0x3f0d5eb] (bss)
00264301: mov    rcx, qword ptr [rbx + 0x30]
00264305: mov    qword ptr [rsp + 0x38], 0
0026430e: mov    byte ptr [rsp + 0x30], 0
00264313: test   rcx, rcx
00264316: je     0x180264387
00264318: mov    r9, qword ptr [rip + 0x3a2e7d9]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
0026431f: lea    r8, [rsp + 0x38]
00264324: mov    rdx, rsi
00264327: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
0026432c: test   al, al
0026432e: je     0x180264373
00264330: mov    rcx, qword ptr [rip + 0x3a5ace9]         ; [0x3cbf020] meta:bool_TypeInfo
00264337: mov    rbx, qword ptr [rsp + 0x38]
0026433c: cmp    dword ptr [rcx + 0xe0], 0
00264343: jne    0x18026434a
00264345: call   0x182f60cf0
0026434a: xor    r8d, r8d
0026434d: lea    rdx, [rsp + 0x30]
00264352: mov    rcx, rbx
00264355: call   0x181a8e310                              ; System.Boolean$$TryParse
0026435a: test   al, al
0026435c: je     0x180264373
0026435e: movzx  eax, byte ptr [rsp + 0x30]
00264363: mov    rbx, qword ptr [rsp + 0x40]
00264368: mov    rsi, qword ptr [rsp + 0x48]
0026436d: add    rsp, 0x20
00264371: pop    rdi
00264372: ret    
00264373: mov    rbx, qword ptr [rsp + 0x40]
00264378: movzx  eax, dil
0026437c: mov    rsi, qword ptr [rsp + 0x48]
00264381: add    rsp, 0x20
00264385: pop    rdi
00264386: ret    
00264387: call   0x182f60c50
0026438c: int3   
0026438d: int3   
