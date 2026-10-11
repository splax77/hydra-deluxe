020f3610: mov    qword ptr [rsp + 8], rbx
020f3615: push   rdi
020f3616: sub    rsp, 0x20
020f361a: cmp    byte ptr [rip + 0x1e25d87], 0            ; [0x3f193a8] (bss)
020f3621: mov    rdi, rdx
020f3624: mov    rbx, rcx
020f3627: jne    0x1820f363c
020f3629: lea    rcx, [rip + 0x1c1b670]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f3630: call   0x182f609b0
020f3635: mov    byte ptr [rip + 0x1e25d6c], 1            ; [0x3f193a8] (bss)
020f363c: test   rdi, rdi
020f363f: je     0x1820f36f6
020f3645: xor    r8d, r8d
020f3648: mov    dl, 1
020f364a: mov    rcx, rdi
020f364d: call   0x18214b680                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʿʸʻʾˁʻʲʶʵʲ
020f3652: xor    edx, edx
020f3654: mov    rcx, rdi
020f3657: call   0x18214bcc0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿˀʼʷʲʸʶʾˀʷʹ
020f365c: test   al, al
020f365e: jne    0x1820f36eb
020f3664: xor    edx, edx
020f3666: mov    rcx, rdi
020f3669: call   0x18214be20                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʶˀʼʼʾʿʽʺʻʵ
020f366e: mov    byte ptr [rbx + 0x46], al
020f3671: xor    r8d, r8d
020f3674: mov    rax, qword ptr [rbx]
020f3677: xor    edx, edx
020f3679: mov    rcx, rbx
020f367c: mov    r9, qword ptr [rax + 0x280]
020f3683: call   qword ptr [rax + 0x278]
020f3689: cmp    dword ptr [rbx + 0x190], 1
020f3690: je     0x1820f36ce
020f3692: xor    edx, edx
020f3694: mov    rcx, rdi
020f3697: call   0x18214b630                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʽʻʻʶˁʼʾˀʽʽ
020f369c: test   al, al
020f369e: je     0x1820f36b0
020f36a0: cmp    byte ptr [rbx + 0x43], 0
020f36a4: jne    0x1820f36b6
020f36a6: xor    edx, edx
020f36a8: mov    rcx, rbx
020f36ab: call   0x1820f62c0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʽˀʺʺʺʷʶʸʶʲʺ
020f36b0: cmp    byte ptr [rbx + 0x43], 0
020f36b4: je     0x1820f36ce
020f36b6: xor    edx, edx
020f36b8: mov    rcx, rdi
020f36bb: call   0x18214b7d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʸʹʲʳʻʾʺʷʲˁʶ
020f36c0: test   al, al
020f36c2: je     0x1820f36ce
020f36c4: xor    edx, edx
020f36c6: mov    rcx, rbx
020f36c9: call   0x1820f7610                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ˁʺʸʿʽʼʸʴʺʹˁ
020f36ce: mov    r8, qword ptr [rbx + 0x1a0]
020f36d5: test   r8, r8
020f36d8: je     0x1820f36eb
020f36da: mov    rdx, qword ptr [rip + 0x1c1b5bf]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f36e1: xor    ecx, ecx
020f36e3: mov    r9, rdi
020f36e6: call   0x182c42260
020f36eb: mov    rbx, qword ptr [rsp + 0x30]
020f36f0: add    rsp, 0x20
020f36f4: pop    rdi
020f36f5: ret    
020f36f6: call   0x182f60c50
020f36fb: int3   
020f36fc: int3   
