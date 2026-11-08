00090940: push   rbx
00090942: sub    rsp, 0x20
00090946: cmp    byte ptr [rip + 0x3e7b732], 0            ; [0x3f0c07f] (bss)
0009094d: mov    rbx, rcx
00090950: jne    0x180090971
00090952: lea    rcx, [rip + 0x3c5ef37]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00090959: call   0x182f609b0
0009095e: lea    rcx, [rip + 0x3c6cd2b]                   ; [0x3cfd690] metamethod:Method$ʲʿʾʲˁʾʸʻʴʻʽ.ʿˀʸʵʾʻʺʵʳʵʺ<ʻʴʻʵʲʵʴʹʷˀʺ>()
00090965: call   0x182f609b0
0009096a: mov    byte ptr [rip + 0x3e7b70e], 1            ; [0x3f0c07f] (bss)
00090971: mov    rcx, qword ptr [rbx + 0x18]
00090975: test   rcx, rcx
00090978: je     0x1800909d3
0009097a: mov    r8, qword ptr [rip + 0x3c6cd0f]          ; [0x3cfd690] metamethod:Method$ʲʿʾʲˁʾʸʻʴʻʽ.ʿˀʸʵʾʻʺʵʳʵʺ<ʻʴʻʵʲʵʴʹʷˀʺ>()
00090981: mov    rdx, qword ptr [rbx + 0x78]
00090985: call   0x1805d7eb0                              ; ʲʿʾʲˁʾʸʻʴʻʽ$$ʿˀʸʵʾʻʺʵʳʵʺ<object>
0009098a: mov    rax, qword ptr [rip + 0x3c5eeff]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00090991: cmp    dword ptr [rax + 0xe0], 0
00090998: jne    0x1800909a9
0009099a: mov    rcx, rax
0009099d: call   0x182f60cf0
000909a2: mov    rax, qword ptr [rip + 0x3c5eee7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000909a9: mov    rax, qword ptr [rax + 0xb8]
000909b0: mov    rcx, qword ptr [rax + 8]
000909b4: test   rcx, rcx
000909b7: je     0x1800909d3
000909b9: mov    rcx, qword ptr [rcx + 0x30]
000909bd: test   rcx, rcx
000909c0: je     0x1800909d3
000909c2: mov    rdx, qword ptr [rbx + 0x78]
000909c6: xor    r8d, r8d
000909c9: add    rsp, 0x20
000909cd: pop    rbx
000909ce: jmp    0x1801f07d0                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ˀʾʿʴʲˀʷʼʹʽʹ
000909d3: call   0x182f60c50
000909d8: int3   
000909d9: int3   
