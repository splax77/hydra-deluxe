021766f0: mov    qword ptr [rsp + 8], rbx
021766f5: push   rdi
021766f6: sub    rsp, 0x20
021766fa: cmp    byte ptr [rip + 0x1da3078], 0            ; [0x3f19779] (bss)
02176701: movzx  edi, dl
02176704: mov    rbx, rcx
02176707: jne    0x18217671c
02176709: lea    rcx, [rip + 0x1b22d10]                   ; [0x3c99420] metamethod:Method$ʽʻʼʶˀˀʶʲʶʵʾ.ˀʼʷʿʺʴʼʸʺʿˁ<ʾʸʸʻʷʸˁʾˁʽʸ, ʿʺʶʶʲˁˁʻʾʻʾ>()
02176710: call   0x182f609b0
02176715: mov    byte ptr [rip + 0x1da305d], 1            ; [0x3f19779] (bss)
0217671c: mov    rcx, qword ptr [rip + 0x1b22cfd]         ; [0x3c99420] metamethod:Method$ʽʻʼʶˀˀʶʲʶʵʾ.ˀʼʷʿʺʴʼʸʺʿˁ<ʾʸʸʻʷʸˁʾˁʽʸ, ʿʺʶʶʲˁˁʻʾʻʾ>()
02176723: cmp    qword ptr [rcx + 0x38], 0
02176728: jne    0x18217672f
0217672a: call   0x182f657d0
0217672f: mov    eax, dword ptr [rbx]
02176731: test   dil, dil
02176734: jne    0x182176747
02176736: btr    eax, 6
0217673a: mov    dword ptr [rbx], eax
0217673c: mov    rbx, qword ptr [rsp + 0x30]
02176741: add    rsp, 0x20
02176745: pop    rdi
02176746: ret    
02176747: bts    eax, 6
0217674b: mov    dword ptr [rbx], eax
0217674d: mov    rbx, qword ptr [rsp + 0x30]
02176752: add    rsp, 0x20
02176756: pop    rdi
02176757: ret    
02176758: int3   
