00235a40: mov    qword ptr [rsp + 8], rbx
00235a45: mov    qword ptr [rsp + 0x10], rsi
00235a4a: push   rdi
00235a4b: sub    rsp, 0x20
00235a4f: mov    rdi, qword ptr [rcx + 0x38]
00235a53: test   rdi, rdi
00235a56: je     0x180235abd
00235a58: xor    ebx, ebx
00235a5a: xor    eax, eax
00235a5c: nop    dword ptr [rax]
00235a60: cmp    eax, dword ptr [rdi + 0x18]
00235a63: jge    0x180235aab
00235a65: cmp    ebx, dword ptr [rdi + 0x18]
00235a68: jae    0x180235ac3
00235a6a: movsxd rax, ebx
00235a6d: mov    rsi, qword ptr [rdi + rax*8 + 0x20]
00235a72: test   rsi, rsi
00235a75: je     0x180235abd
00235a77: xor    edx, edx
00235a79: mov    rcx, rsi
00235a7c: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
00235a81: test   al, al
00235a83: je     0x180235a93
00235a85: xor    edx, edx
00235a87: mov    rcx, rsi
00235a8a: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
00235a8f: test   al, al
00235a91: jne    0x180235a99
00235a93: inc    ebx
00235a95: mov    eax, ebx
00235a97: jmp    0x180235a60
00235a99: mov    al, 1
00235a9b: mov    rbx, qword ptr [rsp + 0x30]
00235aa0: mov    rsi, qword ptr [rsp + 0x38]
00235aa5: add    rsp, 0x20
00235aa9: pop    rdi
00235aaa: ret    
00235aab: mov    rbx, qword ptr [rsp + 0x30]
00235ab0: xor    al, al
00235ab2: mov    rsi, qword ptr [rsp + 0x38]
00235ab7: add    rsp, 0x20
00235abb: pop    rdi
00235abc: ret    
00235abd: call   0x182f60c50
00235ac2: int3   
00235ac3: call   0x182f60c40
00235ac8: int3   
00235ac9: int3   
