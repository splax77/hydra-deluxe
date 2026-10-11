020de770: mov    qword ptr [rsp + 0x10], rbx
020de775: push   rdi
020de776: sub    rsp, 0x20
020de77a: mov    rdi, rdx
020de77d: mov    rbx, rcx
020de780: test   rdx, rdx
020de783: je     0x1820de995
020de789: test   byte ptr [rdx + 0x80], 1
020de790: jne    0x1820de7d7
020de792: cmp    word ptr [rcx + 0x2c4], 0
020de79a: je     0x1820de7d7
020de79c: test   dword ptr [rdx + 0x7c], 0xc000
020de7a3: jne    0x1820de7d7
020de7a5: mov    eax, dword ptr [rcx + 0x2ec]
020de7ab: inc    eax
020de7ad: mov    dword ptr [rcx + 0x2ec], eax
020de7b3: cmp    eax, 3
020de7b6: jb     0x1820de7d7
020de7b8: xor    eax, eax
020de7ba: mov    dword ptr [rcx + 0x2ec], 0
020de7c4: mov    dword ptr [rcx + 0x2c4], eax
020de7ca: mov    byte ptr [rcx + 0x2c8], al
020de7d0: mov    word ptr [rcx + 0x2f0], ax
020de7d7: test   dword ptr [rdx + 0x7c], 0x10000
020de7de: je     0x1820de7f5
020de7e0: mov    byte ptr [rcx + 0x2f1], 1
020de7e7: mov    rax, qword ptr [rcx + 0x100]
020de7ee: mov    qword ptr [rcx + 0x2d8], rax
020de7f5: cmp    word ptr [rdx + 0x80], 2
020de7fd: mov    qword ptr [rsp + 0x30], rsi
020de802: je     0x1820de891
020de808: cmp    word ptr [rdx + 0x80], 4
020de810: je     0x1820de888
020de812: cmp    word ptr [rdx + 0x80], 8
020de81a: je     0x1820de87f
020de81c: cmp    word ptr [rdx + 0x80], 0x10
020de824: je     0x1820de876
020de826: cmp    word ptr [rdx + 0x80], 1
020de82e: je     0x1820de86d
020de830: cmp    word ptr [rdx + 0x80], 0x20
020de838: je     0x1820de864
020de83a: cmp    word ptr [rdx + 0x80], 0x40
020de842: je     0x1820de85b
020de844: mov    eax, 0x80
020de849: cmp    word ptr [rdx + 0x80], ax
020de850: jne    0x1820de891
020de852: movzx  esi, byte ptr [rcx + 0x310]
020de859: jmp    0x1820de898
020de85b: movzx  esi, byte ptr [rcx + 0x30f]
020de862: jmp    0x1820de898
020de864: movzx  esi, byte ptr [rcx + 0x30e]
020de86b: jmp    0x1820de898
020de86d: movzx  esi, byte ptr [rcx + 0x30d]
020de874: jmp    0x1820de898
020de876: movzx  esi, byte ptr [rcx + 0x30c]
020de87d: jmp    0x1820de898
020de87f: movzx  esi, byte ptr [rcx + 0x30b]
020de886: jmp    0x1820de898
020de888: movzx  esi, byte ptr [rcx + 0x30a]
020de88f: jmp    0x1820de898
020de891: movzx  esi, byte ptr [rcx + 0x309]
020de898: xor    edx, edx
020de89a: mov    rcx, rdi
020de89d: call   0x18214b6c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʶʲʵʹʸʺʼʶʵʷ
020de8a2: test   al, al
020de8a4: jne    0x1820de8f6
020de8a6: xor    edx, edx
020de8a8: mov    rcx, rdi
020de8ab: call   0x18214be10                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʴˀʽʳʴʹʻʾʴʵ
020de8b0: test   al, al
020de8b2: je     0x1820de928
020de8b4: mov    ecx, dword ptr [rbx + 0x2c0]
020de8ba: movsx  rax, sil
020de8be: cmp    rax, rcx
020de8c1: jl     0x1820de8dd
020de8c3: test   sil, sil
020de8c6: js     0x1820de8dd
020de8c8: xor    r8d, r8d
020de8cb: mov    dl, 1
020de8cd: mov    rcx, rdi
020de8d0: call   0x18214b740                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʶʽʳʺʸʳʼʲʲʿʵ
020de8d5: inc    dword ptr [rbx + 0xdc]
020de8db: jmp    0x1820de928
020de8dd: test   byte ptr [rbx + 0x198], 0x10
020de8e4: jbe    0x1820de928
020de8e6: mov    rsi, qword ptr [rsp + 0x30]
020de8eb: mov    rbx, qword ptr [rsp + 0x38]
020de8f0: add    rsp, 0x20
020de8f4: pop    rdi
020de8f5: ret    
020de8f6: mov    ecx, dword ptr [rbx + 0x2c0]
020de8fc: movsx  rax, sil
020de900: cmp    rax, rcx
020de903: jge    0x1820de91f
020de905: test   sil, sil
020de908: js     0x1820de91f
020de90a: xor    r8d, r8d
020de90d: mov    dl, 1
020de90f: mov    rcx, rdi
020de912: call   0x18214b740                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʶʽʳʺʸʳʼʲʲʿʵ
020de917: inc    dword ptr [rbx + 0xd8]
020de91d: jmp    0x1820de928
020de91f: test   byte ptr [rbx + 0x198], 0x10
020de926: ja     0x1820de8e6
020de928: xor    edx, edx
020de92a: mov    rcx, rdi
020de92d: call   0x18214bb90                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿʶˁʲˁʸʿʴʼʲˁ
020de932: test   al, al
020de934: je     0x1820de94e
020de936: xor    edx, edx
020de938: mov    rcx, rdi
020de93b: call   0x18214bb40                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʾʽʾʾʵʸʳʶʽˀʻ
020de940: test   al, al
020de942: jne    0x1820de94e
020de944: xor    edx, edx
020de946: mov    rcx, rbx
020de949: call   0x1820f5ff0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʻʶʴˁʺʿʳʲˁʽʹ
020de94e: xor    r8d, r8d
020de951: mov    dl, 1
020de953: mov    rcx, rdi
020de956: call   0x18214bb60                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʾˁʿʵʶˁʻʺˀʸʿ
020de95b: xor    r8d, r8d
020de95e: mov    rdx, rdi
020de961: mov    rcx, rbx
020de964: call   0x1820f63f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʾʼʺʻˀʵʶʸʶʻʳ
020de969: mov    rax, qword ptr [rbx]
020de96c: mov    r8d, 0x32
020de972: mov    rdx, rdi
020de975: mov    rcx, rbx
020de978: mov    r9, qword ptr [rax + 0x1a0]
020de97f: mov    rsi, qword ptr [rsp + 0x30]
020de984: mov    rbx, qword ptr [rsp + 0x38]
020de989: add    rsp, 0x20
020de98d: pop    rdi
020de98e: jmp    qword ptr [rax + 0x198]
020de995: call   0x182f60c50
020de99a: int3   
020de99b: int3   
