0211af60: push   rbx
0211af62: push   rbp
0211af63: push   rdi
0211af64: sub    rsp, 0x20
0211af68: cmp    byte ptr [rip + 0x1dfe5e4], 0            ; [0x3f19553] (bss)
0211af6f: mov    rbx, r9
0211af72: mov    rdi, r8
0211af75: mov    rbp, rcx
0211af78: jne    0x18211afb1
0211af7a: lea    rcx, [rip + 0x1bc887f]                   ; [0x3ce3800] meta:System.Enum_TypeInfo
0211af81: call   0x182f609b0
0211af86: lea    rcx, [rip + 0x1bcb1eb]                   ; [0x3ce6178] meta:System.Type_TypeInfo
0211af8d: call   0x182f609b0
0211af92: lea    rcx, [rip + 0x1bda7b7]                   ; [0x3cf5750] meta:ˀʾʲʳʾʻˁʺʵʾʾ_var
0211af99: call   0x182f609b0
0211af9e: lea    rcx, [rip + 0x1bce273]                   ; [0x3ce9218] meta:ushort_TypeInfo
0211afa5: call   0x182f609b0
0211afaa: mov    byte ptr [rip + 0x1dfe5a2], 1            ; [0x3f19553] (bss)
0211afb1: mov    qword ptr [rsp + 0x40], rsi
0211afb6: test   rdi, rdi
0211afb9: je     0x18211b0d7
0211afbf: xor    r8d, r8d
0211afc2: mov    edx, 2
0211afc7: mov    rcx, rdi
0211afca: call   0x182142500                              ; ʺʵˀʹˀˀʲˁʼʸʸ$$ʹˁˁʴʲʷʻʳʷʵʸ
0211afcf: mov    rsi, qword ptr [rdi + 0x18]
0211afd3: test   rsi, rsi
0211afd6: je     0x18211b0d7
0211afdc: movsxd rax, dword ptr [rdi + 0x14]
0211afe0: cmp    eax, dword ptr [rsi + 0x18]
0211afe3: jae    0x18211b1a5
0211afe9: movzx  esi, word ptr [rax + rsi + 0x20]
0211afee: add    dword ptr [rdi + 0x14], 2
0211aff2: ror    si, 8
0211aff6: test   rbx, rbx
0211aff9: je     0x18211b0d7
0211afff: cmp    dword ptr [rbx + 0x10], 0
0211b003: jne    0x18211b078
0211b005: mov    rcx, qword ptr [rip + 0x1bcb16c]         ; [0x3ce6178] meta:System.Type_TypeInfo
0211b00c: mov    rbx, qword ptr [rip + 0x1bda73d]         ; [0x3cf5750] meta:ˀʾʲʳʾʻˁʺʵʾʾ_var
0211b013: mov    qword ptr [rsp + 0x48], r14
0211b018: cmp    dword ptr [rcx + 0xe0], 0
0211b01f: jne    0x18211b026
0211b021: call   0x182f60cf0
0211b026: xor    edx, edx
0211b028: mov    rcx, rbx
0211b02b: call   0x181b7a070                              ; System.Type$$GetTypeFromHandle
0211b030: mov    rcx, qword ptr [rip + 0x1bce1e1]         ; [0x3ce9218] meta:ushort_TypeInfo
0211b037: lea    rdx, [rsp + 0x50]
0211b03c: mov    rbx, rax
0211b03f: mov    word ptr [rsp + 0x50], si
0211b044: call   0x182f5fbe0
0211b049: mov    rcx, qword ptr [rip + 0x1bc87b0]         ; [0x3ce3800] meta:System.Enum_TypeInfo
0211b050: mov    r14, rax
0211b053: cmp    dword ptr [rcx + 0xe0], 0
0211b05a: jne    0x18211b061
0211b05c: call   0x182f60cf0
0211b061: xor    r8d, r8d
0211b064: mov    rdx, r14
0211b067: mov    rcx, rbx
0211b06a: call   0x181b8e8b0                              ; System.Enum$$IsDefined
0211b06f: mov    r14, qword ptr [rsp + 0x48]
0211b074: test   al, al
0211b076: je     0x18211b0dd
0211b078: xor    r8d, r8d
0211b07b: mov    word ptr [rbp + 0x10], si
0211b07f: mov    edx, 2
0211b084: mov    rcx, rdi
0211b087: call   0x182142500                              ; ʺʵˀʹˀˀʲˁʼʸʸ$$ʹˁˁʴʲʷʻʳʷʵʸ
0211b08c: mov    rcx, qword ptr [rdi + 0x18]
0211b090: test   rcx, rcx
0211b093: je     0x18211b0d7
0211b095: movsxd rax, dword ptr [rdi + 0x14]
0211b099: cmp    eax, dword ptr [rcx + 0x18]
0211b09c: jae    0x18211b1a5
0211b0a2: movzx  ecx, word ptr [rax + rcx + 0x20]
0211b0a7: xor    edx, edx
0211b0a9: add    dword ptr [rdi + 0x14], 2
0211b0ad: ror    cx, 8
0211b0b1: mov    word ptr [rbp + 0x14], cx
0211b0b5: mov    rcx, rdi
0211b0b8: call   0x182142020                              ; ʺʵˀʹˀˀʲˁʼʸʸ$$ʲʺˀʽʾʳʵʼʸʻʴ
0211b0bd: test   ax, ax
0211b0c0: js     0x18211b15c
0211b0c6: mov    rsi, qword ptr [rsp + 0x40]
0211b0cb: mov    word ptr [rbp + 0x12], ax
0211b0cf: add    rsp, 0x20
0211b0d3: pop    rdi
0211b0d4: pop    rbp
0211b0d5: pop    rbx
0211b0d6: ret    
0211b0d7: call   0x182f60c50
0211b0dc: int3   
0211b0dd: lea    rcx, [rip + 0x1bce134]                   ; [0x3ce9218] meta:ushort_TypeInfo
0211b0e4: mov    word ptr [rsp + 0x50], si
0211b0e9: call   0x182f609d0
0211b0ee: mov    rcx, rax
0211b0f1: lea    rdx, [rsp + 0x50]
0211b0f6: call   0x182f5fbe0
0211b0fb: lea    rcx, [rip + 0x1bd7786]                   ; [0x3cf2888] str:'note_anim_sp_phrase'
0211b102: mov    rbx, rax
0211b105: call   0x182f609d0
0211b10a: mov    rcx, rax
0211b10d: xor    r8d, r8d
0211b110: mov    rdx, rbx
0211b113: call   0x181982590                              ; System.String$$Format
0211b118: lea    rcx, [rip + 0x1bd1f39]                   ; [0x3ced058] meta:StrikeCore.DryWetMidi.Smf.UnknownFileFormatException_TypeInfo
0211b11f: mov    rbx, rax
0211b122: call   0x182f609d0
0211b127: mov    rcx, rax
0211b12a: call   0x182f60c00
0211b12f: xor    r9d, r9d
0211b132: movzx  r8d, si
0211b136: mov    rdx, rbx
0211b139: mov    rcx, rax
0211b13c: mov    rdi, rax
0211b13f: call   0x182177490                              ; StrikeCore.DryWetMidi.Smf.UnknownFileFormatException$$.ctor
0211b144: lea    rcx, [rip + 0x1be8bf5]                   ; [0x3d03d40] metamethod:Method$ʶʶʹʴʳʷˀʽʽʾʳ.ʳʽˀʲˀʻʵʵʻʽʳ()
0211b14b: call   0x182f609d0
0211b150: mov    rdx, rax
0211b153: mov    rcx, rdi
0211b156: call   0x182f60c10
0211b15b: int3   
0211b15c: lea    rcx, [rip + 0x1b94dbd]                   ; [0x3caff20] meta:System.NotSupportedException_TypeInfo
0211b163: call   0x182f609d0
0211b168: mov    rcx, rax
0211b16b: call   0x182f60c00
0211b170: lea    rcx, [rip + 0x1b92341]                   ; [0x3cad4b8] str:'sf_note_hopo'
0211b177: mov    rbx, rax
0211b17a: call   0x182f609d0
0211b17f: mov    rdx, rax
0211b182: xor    r8d, r8d
0211b185: mov    rcx, rbx
0211b188: call   0x181b54490                              ; System.NotSupportedException$$.ctor
0211b18d: lea    rcx, [rip + 0x1be8bac]                   ; [0x3d03d40] metamethod:Method$ʶʶʹʴʳʷˀʽʽʾʳ.ʳʽˀʲˀʻʵʵʻʽʳ()
0211b194: call   0x182f609d0
0211b199: mov    rdx, rax
0211b19c: mov    rcx, rbx
0211b19f: call   0x182f60c10
0211b1a4: int3   
0211b1a5: call   0x182f60c40
0211b1aa: int3   
0211b1ab: int3   
