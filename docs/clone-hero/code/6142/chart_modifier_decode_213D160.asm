0213d160: mov    qword ptr [rsp + 8], rbx
0213d165: push   rbp
0213d166: mov    rbp, rsp
0213d169: sub    rsp, 0x20
0213d16d: cmp    byte ptr [rip + 0x1ddc4ee], 0            ; [0x3f19662] (bss)
0213d174: mov    ebx, ecx
0213d176: jne    0x18213d18b
0213d178: lea    rcx, [rip + 0x1babbe1]                   ; [0x3ce8d60] metamethod:Method$System.ValueTuple<ʻˀʳʸʺʵʼʺʹˁʽ, ʾʳʿʽʸʸʷʾʼʴʿ.ʼʺˁʲʴʵʽʶʺˁʴ>..ctor()
0213d17f: call   0x182f609b0
0213d184: mov    byte ptr [rip + 0x1ddc4d7], 1            ; [0x3f19662] (bss)
0213d18b: cmp    ebx, 6
0213d18e: jbe    0x18213d258
0213d194: lea    eax, [rbx - 0x22]
0213d197: cmp    eax, 0xa
0213d19a: ja     0x18213d229
0213d1a0: lea    eax, [rbx - 0x22]
0213d1a3: movsxd rcx, eax
0213d1a6: lea    rdx, [rip - 0x213d1ad]                   ; [0x0] (bss)
0213d1ad: mov    eax, dword ptr [rdx + rcx*4 + 0x213d29c]
0213d1b4: add    rax, rdx
0213d1b7: jmp    rax
0213d1b9: mov    r8d, 0xe
0213d1bf: mov    dl, 9
0213d1c1: jmp    0x18213d275
0213d1c6: mov    r8d, 0xf
0213d1cc: mov    dl, 9
0213d1ce: jmp    0x18213d275
0213d1d3: mov    r8d, 0x10
0213d1d9: mov    dl, 9
0213d1db: jmp    0x18213d275
0213d1e0: mov    r8d, 0x12
0213d1e6: mov    dl, 9
0213d1e8: jmp    0x18213d275
0213d1ed: mov    r8d, 0x11
0213d1f3: mov    dl, 9
0213d1f5: jmp    0x18213d275
0213d1f7: mov    r8d, 0xe
0213d1fd: mov    dl, 0xa
0213d1ff: jmp    0x18213d275
0213d201: mov    r8d, 0xf
0213d207: mov    dl, 0xa
0213d209: jmp    0x18213d275
0213d20b: mov    r8d, 0x10
0213d211: mov    dl, 0xa
0213d213: jmp    0x18213d275
0213d215: mov    r8d, 0x12
0213d21b: mov    dl, 0xa
0213d21d: jmp    0x18213d275
0213d21f: mov    r8d, 0x11
0213d225: mov    dl, 0xa
0213d227: jmp    0x18213d275
0213d229: lea    ecx, [rbx - 0x42]
0213d22c: test   ecx, ecx
0213d22e: je     0x18213d24e
0213d230: sub    ecx, 1
0213d233: je     0x18213d244
0213d235: cmp    ecx, 1
0213d238: jne    0x18213d270
0213d23a: mov    r8d, 0x12
0213d240: mov    dl, 7
0213d242: jmp    0x18213d275
0213d244: mov    r8d, 0x10
0213d24a: mov    dl, 7
0213d24c: jmp    0x18213d275
0213d24e: mov    r8d, 0xf
0213d254: mov    dl, 7
0213d256: jmp    0x18213d275
0213d258: cmp    ebx, 5
0213d25b: jne    0x18213d264
0213d25d: xor    r8d, r8d
0213d260: mov    dl, 4
0213d262: jmp    0x18213d275
0213d264: cmp    ebx, 6
0213d267: jne    0x18213d270
0213d269: xor    r8d, r8d
0213d26c: mov    dl, 5
0213d26e: jmp    0x18213d275
0213d270: xor    r8d, r8d
0213d273: xor    edx, edx
0213d275: mov    r9, qword ptr [rip + 0x1babae4]          ; [0x3ce8d60] metamethod:Method$System.ValueTuple<ʻˀʳʸʺʵʼʺʹˁʽ, ʾʳʿʽʸʸʷʾʼʴʿ.ʼʺˁʲʴʵʽʶʺˁʴ>..ctor()
0213d27c: lea    rcx, [rbp + 0x20]
0213d280: mov    qword ptr [rbp + 0x20], 0
0213d288: call   0x180ce1580                              ; MessagePack.ExtensionHeader$$.ctor
0213d28d: mov    rax, qword ptr [rbp + 0x20]
0213d291: mov    rbx, qword ptr [rsp + 0x30]
0213d296: add    rsp, 0x20
0213d29a: pop    rbp
0213d29b: ret    
