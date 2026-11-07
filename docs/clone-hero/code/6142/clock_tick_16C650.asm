; function 0x16c650 ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʸʶˁʷˁʼʳʶʴʸ next method starts at 0x16c910 bytes 0x2c0
0016c650: push   rbx
0016c652: sub    rsp, 0x40
0016c656: cmp    byte ptr [rip + 0x3da010f], 0            ; [0x3f0c76c] (bss)
0016c65d: mov    rbx, rcx
0016c660: movaps xmmword ptr [rsp + 0x30], xmm6
0016c665: movaps xmmword ptr [rsp + 0x20], xmm7
0016c66a: jne    0x18016c68b
0016c66c: lea    rcx, [rip + 0x3b4f7e5]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c673: call   0x182f609b0
0016c678: lea    rcx, [rip + 0x3b360e9]                   ; [0x3ca2768] meta:System.Math_TypeInfo
0016c67f: call   0x182f609b0
0016c684: mov    byte ptr [rip + 0x3da00e1], 1            ; [0x3f0c76c] (bss)
0016c68b: movss  xmm0, dword ptr [rbx + 0x30]
0016c690: xor    ecx, ecx
0016c692: divss  xmm0, dword ptr [rbx + 0x2c]
0016c697: movsd  xmm6, qword ptr [rbx + 0x40]
0016c69c: cvtps2pd xmm7, xmm0
0016c69f: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
0016c6a4: xor    ecx, ecx
0016c6a6: xorps  xmm1, xmm1
0016c6a9: cvtss2sd xmm1, xmm0
0016c6ad: mulsd  xmm1, xmm7
0016c6b1: addsd  xmm1, xmm6
0016c6b5: movsd  xmm6, qword ptr [rbx + 0x38]
0016c6ba: movsd  qword ptr [rbx + 0x40], xmm1
0016c6bf: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
0016c6c4: xorps  xmm1, xmm1
0016c6c7: cvtss2sd xmm1, xmm0
0016c6cb: mulsd  xmm1, xmm7
0016c6cf: addsd  xmm1, xmm6
0016c6d3: movsd  qword ptr [rbx + 0x38], xmm1
0016c6d8: mov    rcx, qword ptr [rip + 0x3b4f779]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c6df: cmp    dword ptr [rcx + 0xe0], 0
0016c6e6: jne    0x18016c6f4
0016c6e8: call   0x182f60cf0
0016c6ed: mov    rcx, qword ptr [rip + 0x3b4f764]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c6f4: mov    rax, qword ptr [rcx + 0xb8]
0016c6fb: mov    rdx, qword ptr [rax + 8]
0016c6ff: test   rdx, rdx
0016c702: je     0x18016c906
0016c708: cmp    byte ptr [rdx + 0x28], 0
0016c70c: je     0x18016c8f6
0016c712: cmp    dword ptr [rcx + 0xe0], 0
0016c719: jne    0x18016c727
0016c71b: call   0x182f60cf0
0016c720: mov    rcx, qword ptr [rip + 0x3b4f731]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c727: mov    rax, qword ptr [rcx + 0xb8]
0016c72e: mov    rdx, qword ptr [rax + 8]
0016c732: test   rdx, rdx
0016c735: je     0x18016c906
0016c73b: cmp    byte ptr [rdx + 0x78], 0
0016c73f: jne    0x18016c8f6
0016c745: cmp    byte ptr [rip + 0x3da001f], 0            ; [0x3f0c76b] (bss)
0016c74c: jne    0x18016c768
0016c74e: lea    rcx, [rip + 0x3b4f703]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c755: call   0x182f609b0
0016c75a: mov    rcx, qword ptr [rip + 0x3b4f6f7]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c761: mov    byte ptr [rip + 0x3da0003], 1            ; [0x3f0c76b] (bss)
0016c768: cmp    dword ptr [rcx + 0xe0], 0
0016c76f: jne    0x18016c77d
0016c771: call   0x182f60cf0
0016c776: mov    rcx, qword ptr [rip + 0x3b4f6db]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c77d: mov    rax, qword ptr [rcx + 0xb8]
0016c784: mov    rcx, qword ptr [rax + 8]
0016c788: test   rcx, rcx
0016c78b: je     0x18016c906
0016c791: xor    edx, edx
0016c793: call   0x180365f10                              ; BassAudioManager$$ʴʴʳʶʺʴʾʲʸʶˀ
0016c798: movss  xmm1, dword ptr [rbx + 0x2c]
0016c79d: cvtps2pd xmm1, xmm1
0016c7a0: divsd  xmm0, xmm1
0016c7a4: addsd  xmm0, qword ptr [rbx + 0x58]
0016c7a9: comisd xmm0, xmmword ptr [rbx + 0x58]
0016c7ae: movsd  qword ptr [rbx + 0x20], xmm0
0016c7b3: jbe    0x18016c8f6
0016c7b9: movsd  xmm1, qword ptr [rbx + 0x10]
0016c7be: subsd  xmm1, qword ptr [rip + 0x2ef75fa]        ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
0016c7c6: comisd xmm1, xmm0
0016c7ca: jbe    0x18016c8f6
0016c7d0: mov    rax, qword ptr [rip + 0x3b4f681]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c7d7: cmp    dword ptr [rax + 0xe0], 0
0016c7de: jne    0x18016c7ef
0016c7e0: mov    rcx, rax
0016c7e3: call   0x182f60cf0
0016c7e8: mov    rax, qword ptr [rip + 0x3b4f669]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0016c7ef: mov    rax, qword ptr [rax + 0xb8]
0016c7f6: mov    rcx, qword ptr [rax + 8]
0016c7fa: test   rcx, rcx
0016c7fd: je     0x18016c906
0016c803: cmp    byte ptr [rcx + 0x38], 0
0016c807: je     0x18016c8f6
0016c80d: cmp    byte ptr [rbx + 0x28], 0
0016c811: movsd  xmm0, qword ptr [rbx + 0x20]
0016c816: subsd  xmm0, qword ptr [rbx + 0x38]
0016c81b: movdqa xmm7, xmmword ptr [rip + 0x2ef761d]      ; [0x3063e40] xmm dbl=(nan, nan) flt=(nan, nan, nan, nan)
0016c823: je     0x18016c856
0016c825: mov    rcx, qword ptr [rip + 0x3b35f3c]         ; [0x3ca2768] meta:System.Math_TypeInfo
0016c82c: cvtpd2ps xmm6, xmm0
0016c830: cmp    dword ptr [rcx + 0xe0], 0
0016c837: jne    0x18016c83e
0016c839: call   0x182f60cf0
0016c83e: movaps xmm0, xmm6
0016c841: andps  xmm0, xmm7
0016c844: comiss xmm0, dword ptr [rip + 0x2ef8081]        ; [0x30648cc] flt=0.003000000026077032 i=994352038
0016c84b: jbe    0x18016c856
0016c84d: movss  dword ptr [rbx + 0x18], xmm6
0016c852: mov    byte ptr [rbx + 0x28], 0
0016c856: mov    rcx, qword ptr [rip + 0x3b35f0b]         ; [0x3ca2768] meta:System.Math_TypeInfo
0016c85d: movss  xmm6, dword ptr [rbx + 0x18]
0016c862: cmp    dword ptr [rcx + 0xe0], 0
0016c869: jne    0x18016c878
0016c86b: call   0x182f60cf0
0016c870: movdqa xmm7, xmmword ptr [rip + 0x2ef75c8]      ; [0x3063e40] xmm dbl=(nan, nan) flt=(nan, nan, nan, nan)
0016c878: andps  xmm6, xmm7
0016c87b: comiss xmm6, dword ptr [rip + 0x2ef804a]        ; [0x30648cc] flt=0.003000000026077032 i=994352038
0016c882: jbe    0x18016c8f6
0016c884: movss  xmm6, dword ptr [rbx + 0x18]
0016c889: xor    ecx, ecx
0016c88b: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
0016c890: mulss  xmm0, xmm6
0016c894: movaps xmm6, xmm0
0016c897: addss  xmm6, dword ptr [rbx + 0x1c]
0016c89c: cvtss2sd xmm0, xmm0
0016c8a0: movss  dword ptr [rbx + 0x1c], xmm6
0016c8a5: addsd  xmm0, qword ptr [rbx + 0x38]
0016c8aa: movsd  qword ptr [rbx + 0x38], xmm0
0016c8af: mov    rcx, qword ptr [rip + 0x3b35eb2]         ; [0x3ca2768] meta:System.Math_TypeInfo
0016c8b6: cmp    dword ptr [rcx + 0xe0], 0
0016c8bd: jne    0x18016c8c4
0016c8bf: call   0x182f60cf0
0016c8c4: movss  xmm0, dword ptr [rbx + 0x18]
0016c8c9: andps  xmm0, xmmword ptr [rip + 0x2ef7570]      ; [0x3063e40] xmm dbl=(nan, nan) flt=(nan, nan, nan, nan)
0016c8d0: andps  xmm6, xmmword ptr [rip + 0x2ef7569]      ; [0x3063e40] xmm dbl=(nan, nan) flt=(nan, nan, nan, nan)
0016c8d7: comiss xmm6, xmm0
0016c8da: jb     0x18016c8f6
0016c8dc: movss  xmm0, dword ptr [rbx + 0x18]
0016c8e1: subss  xmm0, dword ptr [rbx + 0x1c]
0016c8e6: mov    dword ptr [rbx + 0x1c], 0
0016c8ed: mov    byte ptr [rbx + 0x28], 1
0016c8f1: movss  dword ptr [rbx + 0x18], xmm0
0016c8f6: movaps xmm6, xmmword ptr [rsp + 0x30]
0016c8fb: movaps xmm7, xmmword ptr [rsp + 0x20]
0016c900: add    rsp, 0x40
0016c904: pop    rbx
0016c905: ret    
0016c906: call   0x182f60c50
0016c90b: int3   
0016c90c: int3   
0016c90d: int3   
0016c90e: int3   
0016c90f: int3   
