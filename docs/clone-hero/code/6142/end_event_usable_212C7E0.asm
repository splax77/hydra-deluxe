0212c7e0: sub    rsp, 0x28
0212c7e4: cmp    qword ptr [rcx + 0x108], 0
0212c7ec: jne    0x18212c7f5
0212c7ee: xor    al, al
0212c7f0: add    rsp, 0x28
0212c7f4: ret    
0212c7f5: mov    rax, qword ptr [rcx + 0x108]
0212c7fc: xor    r8d, r8d
0212c7ff: mov    qword ptr [rsp + 0x20], rbx
0212c804: mov    dl, 1
0212c806: mov    rbx, qword ptr [rax + 0x10]
0212c80a: call   0x18212e130                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʿʼʿʿʵʵʲʿʷˁʻ
0212c80f: cmp    rbx, rax
0212c812: mov    rbx, qword ptr [rsp + 0x20]
0212c817: setge  al
0212c81a: add    rsp, 0x28
0212c81e: ret    
0212c81f: int3   
