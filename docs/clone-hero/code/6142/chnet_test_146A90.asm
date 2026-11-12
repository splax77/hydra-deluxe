00146a90: push   rbx
00146a92: sub    rsp, 0x20
00146a96: cmp    byte ptr [rip + 0x3dc5c0b], 0            ; [0x3f0c6a8] (bss)
00146a9d: mov    rbx, rcx
00146aa0: jne    0x180146ab5
00146aa2: lea    rcx, [rip + 0x3bc859f]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
00146aa9: call   0x182f609b0
00146aae: mov    byte ptr [rip + 0x3dc5bf3], 1            ; [0x3f0c6a8] (bss)
00146ab5: cmp    byte ptr [rbx + 0x38], 2
00146ab9: je     0x180146ac3
00146abb: xor    al, al
00146abd: add    rsp, 0x20
00146ac1: pop    rbx
00146ac2: ret    
00146ac3: mov    rax, qword ptr [rip + 0x3bc857e]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
00146aca: mov    rcx, qword ptr [rax + 0xb8]
00146ad1: mov    rcx, qword ptr [rcx]
00146ad4: test   rcx, rcx
00146ad7: je     0x180146ae5
00146ad9: xor    edx, edx
00146adb: add    rsp, 0x20
00146adf: pop    rbx
00146ae0: jmp    0x180090610                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʶʳʵʴʷˁʾʾˁʸʳ
00146ae5: call   0x182f60c50
00146aea: int3   
00146aeb: int3   
