02115760: push   rbx
02115762: sub    rsp, 0x20
02115766: cmp    byte ptr [rip + 0x1e03dad], 0            ; [0x3f1951a] (bss)
0211576d: mov    rbx, rcx
02115770: jne    0x18211579d
02115772: lea    rcx, [rip + 0x1bd083f]                   ; [0x3ce5fb8] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.TryGetValue()
02115779: call   0x182f609b0
0211577e: lea    rcx, [rip + 0x1bf87e3]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02115785: call   0x182f609b0
0211578a: lea    rcx, [rip + 0x1bbc587]                   ; [0x3cd1d18] str:'%internal%'
02115791: call   0x182f609b0
02115796: mov    byte ptr [rip + 0x1e03d7d], 1            ; [0x3f1951a] (bss)
0211579d: mov    rax, qword ptr [rip + 0x1bf87c4]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
021157a4: mov    qword ptr [rsp + 0x30], 0
021157ad: cmp    dword ptr [rax + 0xe0], 0
021157b4: jne    0x1821157c5
021157b6: mov    rcx, rax
021157b9: call   0x182f60cf0
021157be: mov    rax, qword ptr [rip + 0x1bf87a3]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
021157c5: mov    rax, qword ptr [rax + 0xb8]
021157cc: mov    rcx, qword ptr [rax + 8]
021157d0: test   rcx, rcx
021157d3: je     0x182115818
021157d5: mov    r9, qword ptr [rip + 0x1bd07dc]          ; [0x3ce5fb8] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.TryGetValue()
021157dc: lea    r8, [rsp + 0x30]
021157e1: mov    rdx, rbx
021157e4: call   0x181316fc0                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$TryGetValue
021157e9: test   al, al
021157eb: je     0x182115810
021157ed: mov    rdx, qword ptr [rip + 0x1bbc524]         ; [0x3cd1d18] str:'%internal%'
021157f4: xor    r8d, r8d
021157f7: mov    rcx, qword ptr [rsp + 0x30]
021157fc: call   0x181989ec0                              ; System.String$$op_Inequality
02115801: test   al, al
02115803: je     0x182115810
02115805: mov    rax, qword ptr [rsp + 0x30]
0211580a: add    rsp, 0x20
0211580e: pop    rbx
0211580f: ret    
02115810: xor    eax, eax
02115812: add    rsp, 0x20
02115816: pop    rbx
02115817: ret    
02115818: call   0x182f60c50
0211581d: int3   
0211581e: int3   
