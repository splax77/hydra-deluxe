; function 0x21647a0 ʼʿʺʿʹʳʹʶʹʹʼ$$ʲʿʿʺʶʻʿʻʵʺʹ next method starts at 0x21648d0 bytes 0x130
021647a0: mov    qword ptr [rsp + 8], rbx
021647a5: mov    qword ptr [rsp + 0x20], rbp
021647aa: push   rsi
021647ab: push   rdi
021647ac: push   r14
021647ae: sub    rsp, 0x40
021647b2: cmp    byte ptr [rip + 0x1db4f63], 0            ; [0x3f1971c] (bss)
021647b9: mov    ebx, r9d
021647bc: mov    rdi, r8
021647bf: mov    rsi, rdx
021647c2: mov    rbp, rcx
021647c5: jne    0x1821647e6
021647c7: lea    rcx, [rip + 0x1b6be62]                   ; [0x3cd0630] meta:System.Globalization.CultureInfo_TypeInfo
021647ce: call   0x182f609b0
021647d3: lea    rcx, [rip + 0x1b87c9e]                   ; [0x3cec478] str:'.'
021647da: call   0x182f609b0
021647df: mov    byte ptr [rip + 0x1db4f36], 1            ; [0x3f1971c] (bss)
021647e6: xor    r14d, r14d
021647e9: lea    r9, [rsp + 0x30]
021647ee: mov    r8, rdi
021647f1: mov    qword ptr [rsp + 0x30], r14
021647f6: mov    rdx, rsi
021647f9: mov    dword ptr [rsp + 0x70], r14d
021647fe: mov    rcx, rbp
02164801: mov    dword ptr [rsp + 0x68], r14d
02164806: mov    qword ptr [rsp + 0x20], r14
0216480b: call   0x1801bd150
02164810: test   al, al
02164812: je     0x1821648a8
02164818: mov    rcx, qword ptr [rsp + 0x30]
0216481d: test   rcx, rcx
02164820: je     0x1821648bd
02164826: mov    rdx, qword ptr [rip + 0x1b87c4b]         ; [0x3cec478] str:'.'
0216482d: xor    r8d, r8d
02164830: call   0x18197f670                              ; System.String$$Contains
02164835: test   al, al
02164837: jne    0x182164855
02164839: mov    rcx, qword ptr [rsp + 0x30]
0216483e: lea    rdx, [rsp + 0x68]
02164843: xor    r8d, r8d
02164846: call   0x181b4f710                              ; System.Int32$$TryParse
0216484b: test   al, al
0216484d: je     0x1821648a8
0216484f: mov    eax, dword ptr [rsp + 0x68]
02164853: jmp    0x1821648aa
02164855: mov    rcx, qword ptr [rip + 0x1b6bdd4]         ; [0x3cd0630] meta:System.Globalization.CultureInfo_TypeInfo
0216485c: mov    rdi, qword ptr [rsp + 0x30]
02164861: cmp    dword ptr [rcx + 0xe0], r14d
02164868: jne    0x18216486f
0216486a: call   0x182f60cf0
0216486f: xor    ecx, ecx
02164871: call   0x181acf580                              ; System.Globalization.CultureInfo$$get_InvariantCulture
02164876: lea    r9, [rsp + 0x70]
0216487b: mov    qword ptr [rsp + 0x20], r14
02164880: mov    r8, rax
02164883: mov    edx, 0xa7
02164888: mov    rcx, rdi
0216488b: call   0x181b70450                              ; System.Single$$TryParse
02164890: test   al, al
02164892: je     0x1821648a8
02164894: movss  xmm0, dword ptr [rsp + 0x70]
0216489a: mulss  xmm0, dword ptr [rip + 0xf00062]         ; [0x3064904] flt=1000.0 i=1148846080
021648a2: cvttss2si eax, xmm0
021648a6: jmp    0x1821648aa
021648a8: mov    eax, ebx
021648aa: mov    rbx, qword ptr [rsp + 0x60]
021648af: mov    rbp, qword ptr [rsp + 0x78]
021648b4: add    rsp, 0x40
021648b8: pop    r14
021648ba: pop    rdi
021648bb: pop    rsi
021648bc: ret    
021648bd: call   0x182f60c50
021648c2: int3   
021648c3: int3   
021648c4: int3   
021648c5: int3   
021648c6: int3   
021648c7: int3   
021648c8: int3   
021648c9: int3   
021648ca: int3   
021648cb: int3   
021648cc: int3   
021648cd: int3   
021648ce: int3   
021648cf: int3   
