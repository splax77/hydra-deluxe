02176900: mov    qword ptr [rsp + 8], rbx
02176905: push   rdi
02176906: sub    rsp, 0x20
0217690a: cmp    byte ptr [rip + 0x1da2e66], 0            ; [0x3f19777] (bss)
02176911: movzx  edi, dl
02176914: mov    rbx, rcx
02176917: jne    0x18217692c
02176919: lea    rcx, [rip + 0x1b22b00]                   ; [0x3c99420] metamethod:Method$ʽʻʼʶˀˀʶʲʶʵʾ.ˀʼʷʿʺʴʼʸʺʿˁ<ʾʸʸʻʷʸˁʾˁʽʸ, ʿʺʶʶʲˁˁʻʾʻʾ>()
02176920: call   0x182f609b0
02176925: mov    byte ptr [rip + 0x1da2e4b], 1            ; [0x3f19777] (bss)
0217692c: mov    rcx, qword ptr [rip + 0x1b22aed]         ; [0x3c99420] metamethod:Method$ʽʻʼʶˀˀʶʲʶʵʾ.ˀʼʷʿʺʴʼʸʺʿˁ<ʾʸʸʻʷʸˁʾˁʽʸ, ʿʺʶʶʲˁˁʻʾʻʾ>()
02176933: cmp    qword ptr [rcx + 0x38], 0
02176938: jne    0x18217693f
0217693a: call   0x182f657d0
0217693f: mov    eax, dword ptr [rbx]
02176941: test   dil, dil
02176944: jne    0x182176957
02176946: btr    eax, 2
0217694a: mov    dword ptr [rbx], eax
0217694c: mov    rbx, qword ptr [rsp + 0x30]
02176951: add    rsp, 0x20
02176955: pop    rdi
02176956: ret    
02176957: bts    eax, 2
0217695b: mov    dword ptr [rbx], eax
0217695d: mov    rbx, qword ptr [rsp + 0x30]
02176962: add    rsp, 0x20
02176966: pop    rdi
02176967: ret    
02176968: int3   
