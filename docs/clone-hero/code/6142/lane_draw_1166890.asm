01166890: mov    qword ptr [rsp + 0x20], rbp
01166895: push   rsi
01166896: sub    rsp, 0x60
0116689a: cmp    byte ptr [rip + 0x2dab01c], 0            ; [0x3f118bd] (bss)
011668a1: mov    rbp, rdx
011668a4: mov    rsi, rcx
011668a7: jne    0x1811668bc
011668a9: lea    rcx, [rip + 0x2b6e538]                   ; [0x3cd4de8] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.get_Count()
011668b0: call   0x182f609b0
011668b5: mov    byte ptr [rip + 0x2dab001], 1            ; [0x3f118bd] (bss)
011668bc: mov    rax, qword ptr [rsi + 0x20]
011668c0: mov    qword ptr [rsp + 0x78], rbx
011668c5: mov    qword ptr [rsp + 0x80], rdi
011668cd: movaps xmmword ptr [rsp + 0x50], xmm6
011668d2: test   rax, rax
011668d5: je     0x181166ac4
011668db: mov    rcx, qword ptr [rax + 0x120]
011668e2: test   rcx, rcx
011668e5: je     0x181166ac4
011668eb: xor    edx, edx
011668ed: call   0x1820f4b50                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʶʹʻʳʵʽʿʹʹʸʲ
011668f2: test   rax, rax
011668f5: je     0x181166931
011668f7: mov    rax, qword ptr [rsi + 0x20]
011668fb: test   rax, rax
011668fe: je     0x181166ac4
01166904: mov    rcx, qword ptr [rax + 0x120]
0116690b: test   rcx, rcx
0116690e: je     0x181166ac4
01166914: xor    edx, edx
01166916: call   0x1820f4b50                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʶʹʻʳʵʽʿʹʹʸʲ
0116691b: lea    rcx, [rsi + 0xe0]
01166922: mov    qword ptr [rsi + 0xe0], rax
01166929: mov    rdx, rax
0116692c: call   0x182f5fc00
01166931: mov    rax, qword ptr [rsi + 0x20]
01166935: test   rax, rax
01166938: je     0x181166ac4
0116693e: mov    rcx, qword ptr [rax + 0x120]
01166945: test   rcx, rcx
01166948: je     0x181166ac4
0116694e: xor    edx, edx
01166950: xor    edi, edi
01166952: call   0x1820f4e20                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʷʸʲʳʾʸʴʷʿʻʹ
01166957: mov    rcx, qword ptr [rbp + 0x20]
0116695b: lea    r8, [rsi + 0x70]
0116695f: mov    rdx, rax
01166962: mov    r9, qword ptr [rcx + 0xc0]
01166969: mov    rcx, rsi
0116696c: mov    r9, qword ptr [r9 + 0xc0]
01166973: call   0x18041b5c0                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʸʽʿʲʵʹʷʷʸʵʲ<object>
01166978: movss  xmm6, dword ptr [rip + 0x1efdb68]        ; [0x30644e8] flt=1.25 i=1067450368
01166980: mov    rbx, rax
01166983: mov    qword ptr [rsp + 0x70], rax
01166988: cmp    eax, -1
0116698b: je     0x1811669f6
0116698d: mov    rax, qword ptr [rsi + 0x20]
01166991: test   rax, rax
01166994: je     0x181166ac4
0116699a: mov    rcx, qword ptr [rax + 0x120]
011669a1: test   rcx, rcx
011669a4: je     0x181166ac4
011669aa: xor    edx, edx
011669ac: call   0x1820f4e20                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʷʸʲʳʾʸʴʷʿʻʹ
011669b1: mov    rcx, qword ptr [rbp + 0x20]
011669b5: mov    r9, qword ptr [rsi + 0x78]
011669b9: mov    r8, qword ptr [rsi + 0xa0]
011669c0: mov    rdx, qword ptr [rcx + 0xc0]
011669c7: mov    rcx, qword ptr [rdx + 0xc8]
011669ce: mov    rdx, rax
011669d1: mov    qword ptr [rsp + 0x40], rcx
011669d6: mov    ecx, dword ptr [rsp + 0x74]
011669da: movss  dword ptr [rsp + 0x38], xmm6
011669e0: mov    dword ptr [rsp + 0x30], edi
011669e4: mov    dword ptr [rsp + 0x28], ecx
011669e8: mov    rcx, rsi
011669eb: mov    dword ptr [rsp + 0x20], ebx
011669ef: call   0x18041be70                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʹʴʶʽʶʹʿʷˀʳʾ<object>
011669f4: mov    edi, eax
011669f6: cmp    qword ptr [rsi + 0xe0], 0
011669fe: je     0x181166a5a
01166a00: mov    rcx, qword ptr [rsi + 0xe0]
01166a07: cmp    dword ptr [rcx + 0x18], 0
01166a0b: jle    0x181166a5a
01166a0d: mov    rcx, qword ptr [rbp + 0x20]
01166a11: mov    r9, qword ptr [rsi + 0x78]
01166a15: mov    r8, qword ptr [rsi + 0xa0]
01166a1c: mov    rdx, qword ptr [rcx + 0xc0]
01166a23: mov    rcx, qword ptr [rdx + 0xd0]
01166a2a: mov    rdx, qword ptr [rsi + 0xe0]
01166a31: mov    qword ptr [rsp + 0x40], rcx
01166a36: mov    rcx, rsi
01166a39: movss  dword ptr [rsp + 0x38], xmm6
01166a3f: mov    dword ptr [rsp + 0x30], edi
01166a43: mov    dword ptr [rsp + 0x28], 1
01166a4b: mov    dword ptr [rsp + 0x20], 0
01166a53: call   0x18041be70                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʹʴʶʽʶʹʿʷˀʳʾ<object>
01166a58: mov    edi, eax
01166a5a: mov    rsi, qword ptr [rsi + 0xa0]
01166a61: test   rsi, rsi
01166a64: je     0x181166ac4
01166a66: mov    eax, edi
01166a68: cmp    eax, dword ptr [rsi + 0x18]
01166a6b: jge    0x181166a9e
01166a6d: cmp    edi, dword ptr [rsi + 0x18]
01166a70: jae    0x181166abe
01166a72: movsxd rax, edi
01166a75: mov    rbx, qword ptr [rsi + rax*8 + 0x20]
01166a7a: test   rbx, rbx
01166a7d: je     0x181166ac4
01166a7f: xor    edx, edx
01166a81: mov    rcx, rbx
01166a84: call   0x18287b320                              ; UnityEngine.GameObject$$get_activeSelf
01166a89: test   al, al
01166a8b: je     0x181166a9e
01166a8d: xor    r8d, r8d
01166a90: xor    edx, edx
01166a92: mov    rcx, rbx
01166a95: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
01166a9a: inc    edi
01166a9c: jmp    0x181166a66
01166a9e: movaps xmm6, xmmword ptr [rsp + 0x50]
01166aa3: mov    rdi, qword ptr [rsp + 0x80]
01166aab: mov    rbx, qword ptr [rsp + 0x78]
01166ab0: mov    rbp, qword ptr [rsp + 0x88]
01166ab8: add    rsp, 0x60
01166abc: pop    rsi
01166abd: ret    
01166abe: call   0x182f60c40
01166ac3: int3   
01166ac4: call   0x182f60c50
01166ac9: int3   
01166aca: int3   
