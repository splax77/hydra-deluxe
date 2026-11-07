011676a0: mov    qword ptr [rsp + 0x10], rbx
011676a5: push   rdi
011676a6: sub    rsp, 0x20
011676aa: cmp    byte ptr [rip + 0x2daa22a], 0            ; [0x3f118db] (bss)
011676b1: mov    rdi, rdx
011676b4: mov    rbx, rcx
011676b7: jne    0x1811676fc
011676b9: lea    rcx, [rip + 0x2b5c158]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
011676c0: call   0x182f609b0
011676c5: lea    rcx, [rip + 0x2b881c4]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
011676cc: call   0x182f609b0
011676d1: lea    rcx, [rip + 0x2b30c48]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
011676d8: call   0x182f609b0
011676dd: lea    rcx, [rip + 0x2b341cc]                   ; [0x3c9b8b0] meta:ʼˁʿʷʶʸʴʼʹʾʶ_TypeInfo
011676e4: call   0x182f609b0
011676e9: lea    rcx, [rip + 0x2ba2a08]                   ; [0x3d0a0f8] str:'{0}Engine'
011676f0: call   0x182f609b0
011676f5: mov    byte ptr [rip + 0x2daa1df], 1            ; [0x3f118db] (bss)
011676fc: mov    qword ptr [rbx + 8], 0
01167704: mov    rdx, rdi
01167707: mov    qword ptr [rbx + 0x10], 0
0116770f: mov    rcx, rbx
01167712: mov    qword ptr [rbx + 0x18], 0
0116771a: mov    qword ptr [rbx], rdi
0116771d: call   0x182f5fc00
01167722: mov    rax, qword ptr [rdi + 0x88]
01167729: test   rax, rax
0116772c: je     0x181167940
01167732: mov    rax, qword ptr [rax + 0x10]
01167736: test   rax, rax
01167739: je     0x181167940
0116773f: mov    eax, dword ptr [rax + 0x14]
01167742: mov    dword ptr [rbx + 8], eax
01167745: mov    rax, qword ptr [rip + 0x2b88144]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0116774c: cmp    dword ptr [rax + 0xe0], 0
01167753: jne    0x181167764
01167755: mov    rcx, rax
01167758: call   0x182f60cf0
0116775d: mov    rax, qword ptr [rip + 0x2b8812c]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
01167764: mov    rax, qword ptr [rax + 0xb8]
0116776b: mov    rcx, qword ptr [rax + 8]
0116776f: test   rcx, rcx
01167772: je     0x181167940
01167778: cmp    byte ptr [rcx + 0x72], 0
0116777c: jne    0x181167803
01167782: mov    rax, qword ptr [rip + 0x2b30b97]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
01167789: xor    edx, edx
0116778b: mov    rcx, qword ptr [rax + 0xb8]
01167792: mov    rcx, qword ptr [rcx + 0xe8]
01167799: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
0116779e: test   al, al
011677a0: jne    0x181167803
011677a2: mov    rax, qword ptr [rip + 0x2b5c06f]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
011677a9: mov    rcx, qword ptr [rax + 0xb8]
011677b0: mov    rcx, qword ptr [rcx]
011677b3: test   rcx, rcx
011677b6: je     0x181167940
011677bc: xor    edx, edx
011677be: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
011677c3: test   al, al
011677c5: jne    0x181167803
011677c7: mov    rax, qword ptr [rip + 0x2b880c2]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
011677ce: cmp    dword ptr [rax + 0xe0], 0
011677d5: jne    0x1811677e6
011677d7: mov    rcx, rax
011677da: call   0x182f60cf0
011677df: mov    rax, qword ptr [rip + 0x2b880aa]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
011677e6: mov    rax, qword ptr [rax + 0xb8]
011677ed: mov    rcx, qword ptr [rax + 8]
011677f1: test   rcx, rcx
011677f4: je     0x181167940
011677fa: cmp    dword ptr [rcx + 0x50], 2
011677fe: setne  al
01167801: jmp    0x181167805
01167803: xor    eax, eax
01167805: mov    byte ptr [rbx + 0xc], al
01167808: mov    rax, qword ptr [rip + 0x2b88081]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0116780f: cmp    dword ptr [rax + 0xe0], 0
01167816: jne    0x181167827
01167818: mov    rcx, rax
0116781b: call   0x182f60cf0
01167820: mov    rax, qword ptr [rip + 0x2b88069]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
01167827: mov    rax, qword ptr [rax + 0xb8]
0116782e: mov    rcx, qword ptr [rax + 8]
01167832: test   rcx, rcx
01167835: je     0x181167940
0116783b: cmp    byte ptr [rcx + 0x72], 0
0116783f: sete   al
01167842: xor    edx, edx
01167844: mov    byte ptr [rbx + 0xd], al
01167847: mov    rcx, qword ptr [rdi + 0x88]
0116784e: call   0x18013f8e0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʾʻʷʿʿʶʵʻʲʷ
01167853: test   al, al
01167855: je     0x18116785b
01167857: xor    al, al
01167859: jmp    0x181167888
0116785b: mov    rcx, qword ptr [rdi + 0x88]
01167862: xor    edx, edx
01167864: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
01167869: test   al, al
0116786b: je     0x181167871
0116786d: mov    al, 1
0116786f: jmp    0x181167888
01167871: mov    rcx, qword ptr [rdi + 0x88]
01167878: test   rcx, rcx
0116787b: je     0x181167940
01167881: xor    edx, edx
01167883: call   0x180337720                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʿʹʲʸʻʻʼʵʸʵˀ
01167888: mov    byte ptr [rbx + 0xe], al
0116788b: mov    rcx, qword ptr [rdi + 0x88]
01167892: test   rcx, rcx
01167895: je     0x181167940
0116789b: xor    edx, edx
0116789d: call   0x180337720                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʿʹʲʸʻʻʼʵʸʵˀ
011678a2: test   al, al
011678a4: jne    0x1811678db
011678a6: mov    rcx, qword ptr [rdi + 0x88]
011678ad: xor    edx, edx
011678af: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
011678b4: test   al, al
011678b6: jne    0x1811678d2
011678b8: mov    rcx, qword ptr [rdi + 0x88]
011678bf: xor    edx, edx
011678c1: call   0x18013f8e0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʾʻʷʿʿʶʵʻʲʷ
011678c6: neg    al
011678c8: sbb    ecx, ecx
011678ca: and    ecx, 3
011678cd: mov    dword ptr [rbx + 0x10], ecx
011678d0: jmp    0x1811678e2
011678d2: mov    dword ptr [rbx + 0x10], 2
011678d9: jmp    0x1811678e2
011678db: mov    dword ptr [rbx + 0x10], 1
011678e2: mov    rax, qword ptr [rdi + 0x88]
011678e9: test   rax, rax
011678ec: je     0x181167946
011678ee: mov    rax, qword ptr [rax + 0x10]
011678f2: test   rax, rax
011678f5: je     0x181167946
011678f7: movzx  eax, byte ptr [rax + 0x10]
011678fb: lea    rdx, [rsp + 0x30]
01167900: mov    rcx, qword ptr [rip + 0x2b33fa9]         ; [0x3c9b8b0] meta:ʼˁʿʷʶʸʴʼʹʾʶ_TypeInfo
01167907: mov    byte ptr [rsp + 0x30], al
0116790b: call   0x182f5fbe0
01167910: mov    rcx, qword ptr [rip + 0x2ba27e1]         ; [0x3d0a0f8] str:'{0}Engine'
01167917: xor    r8d, r8d
0116791a: mov    rdx, rax
0116791d: call   0x181982590                              ; System.String$$Format
01167922: lea    rcx, [rbx + 0x18]
01167926: mov    qword ptr [rbx + 0x18], rax
0116792a: mov    rdx, rax
0116792d: call   0x182f5fc00
01167932: mov    rax, rbx
01167935: mov    rbx, qword ptr [rsp + 0x38]
0116793a: add    rsp, 0x20
0116793e: pop    rdi
0116793f: ret    
01167940: call   0x182f60c50
01167945: int3   
01167946: call   0x182f60c50
0116794b: int3   
0116794c: int3   
0116794d: int3   
0116794e: int3   
0116794f: int3   
