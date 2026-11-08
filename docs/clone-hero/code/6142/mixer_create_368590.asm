00368590: mov    qword ptr [rsp + 8], rbx
00368595: push   rdi
00368596: sub    rsp, 0x20
0036859a: cmp    byte ptr [rip + 0x3ba5b46], 0            ; [0x3f0e0e7] (bss)
003685a1: mov    edi, edx
003685a3: mov    rbx, rcx
003685a6: jne    0x1803685c7
003685a8: lea    rcx, [rip + 0x39538a9]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
003685af: call   0x182f609b0
003685b4: lea    rcx, [rip + 0x395d145]                   ; [0x3cc5700] str:'Failed to set mixer buffer length'
003685bb: call   0x182f609b0
003685c0: mov    byte ptr [rip + 0x3ba5b20], 1            ; [0x3f0e0e7] (bss)
003685c7: mov    eax, 0x100
003685cc: mov    r8d, 0x200100
003685d2: cmp    edi, -1
003685d5: mov    edx, 2
003685da: mov    ecx, 0xbb80
003685df: cmove  r8d, eax
003685e3: xor    r9d, r9d
003685e6: call   0x18034c070                              ; ʽʺʸʳʷʲˁʳʷʵʳ$$ʳʶʻʹˀʹˁʿˁʶʴ
003685eb: cmp    edi, -1
003685ee: je     0x180368604
003685f0: movd   xmm2, edi
003685f4: xor    r9d, r9d
003685f7: cvtdq2ps xmm2, xmm2
003685fa: mov    edx, eax
003685fc: mov    rcx, rbx
003685ff: call   0x180366fc0                              ; BassAudioManager$$ʵˀʸʼʶʸʶʸʹʷʴ
00368604: test   rbx, rbx
00368607: je     0x180368644
00368609: mov    dword ptr [rbx + 0x58], eax
0036860c: mov    rcx, qword ptr [rip + 0x3953845]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
00368613: cmp    dword ptr [rcx + 0xe0], 0
0036861a: jne    0x180368621
0036861c: call   0x182f60cf0
00368621: mov    r8, qword ptr [rip + 0x395d0d8]          ; [0x3cc5700] str:'Failed to set mixer buffer length'
00368628: xor    r9d, r9d
0036862b: mov    edx, 1
00368630: mov    ecx, 0x10601
00368635: mov    rbx, qword ptr [rsp + 0x30]
0036863a: add    rsp, 0x20
0036863e: pop    rdi
0036863f: jmp    0x18036afd0                              ; BassAudioManager$$ʼʻˁʿʾʴʳʼʶˁʺ
00368644: call   0x182f60c50
00368649: int3   
0036864a: int3   
