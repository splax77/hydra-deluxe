001f2260: push   rbp
001f2262: push   rbx
001f2263: push   rsi
001f2264: push   rdi
001f2265: push   r12
001f2267: push   r13
001f2269: push   r14
001f226b: push   r15
001f226d: sub    rsp, 0x28
001f2271: lea    rbp, [rsp + 0x20]
001f2276: cmp    byte ptr [rip + 0x3d1a87e], 0            ; [0x3f0cafb] (bss)
001f227d: mov    r13, rdx
001f2280: mov    esi, r8d
001f2283: mov    rdi, rcx
001f2286: jne    0x1801f229b
001f2288: lea    rcx, [rip + 0x3aa9421]                   ; [0x3c9b6b0] metamethod:Method$ʿʳʶʳʾˀʺʸʹʳʽ.ʾʵʿʴʹʺʶʻʶʲʶ()
001f228f: call   0x182f609b0
001f2294: mov    byte ptr [rip + 0x3d1a860], 1            ; [0x3f0cafb] (bss)
001f229b: movzx  ebx, byte ptr [rdi + 0x48]
001f229f: call   0x182f60ba0
001f22a4: test   bl, bl
001f22a6: jne    0x1801f23b4
001f22ac: mov    rax, qword ptr [rdi + 0x38]
001f22b0: cmp    qword ptr [rdi + 0x40], rax
001f22b4: jge    0x1801f23b4
001f22ba: mov    rcx, qword ptr [rdi + 0x40]
001f22be: movabs rax, 0x7fffffffffffffff
001f22c8: sub    rax, rsi
001f22cb: cmp    rcx, rax
001f22ce: jg     0x1801f23f7
001f22d4: test   rcx, rcx
001f22d7: jns    0x1801f22ef
001f22d9: movabs rax, 0x8000000000000000
001f22e3: sub    rax, rcx
001f22e6: cmp    rsi, rax
001f22e9: jl     0x1801f23f7
001f22ef: lea    rax, [rcx + rsi]
001f22f3: cmp    rax, qword ptr [rdi + 0x38]
001f22f7: jle    0x1801f2329
001f22f9: mov    rsi, qword ptr [rdi + 0x38]
001f22fd: mov    rbx, rcx
001f2300: mov    rdx, rcx
001f2303: mov    rcx, rsi
001f2306: call   0x182c4ba90
001f230b: test   al, al
001f230d: jne    0x1801f23c7
001f2313: mov    rax, rsi
001f2316: mov    ecx, 0xffffffff
001f231b: sub    rax, rbx
001f231e: cmp    rax, rcx
001f2321: jg     0x1801f23df
001f2327: sub    esi, ebx
001f2329: mov    rax, 0xfffffffffffffff0
001f2330: call   0x18302b960
001f2335: sub    rsp, rax
001f2338: xor    edx, edx
001f233a: mov    r8, 0xffffffffffffffe6
001f2341: lea    r14, [rsp + 0x20]
001f2346: mov    rcx, r14
001f2349: call   0x18305d9d0
001f234e: xor    r8d, r8d
001f2351: mov    rdx, r14
001f2354: mov    rcx, rdi
001f2357: call   0x1801f1d50                              ; ʿʳʶʳʾˀʺʸʹʳʽ$$ʵʹʾʹʲʼʵˀʹʶʴ
001f235c: mov    rbx, qword ptr [rdi + 0x40]
001f2360: xor    r15d, r15d
001f2363: and    rbx, 0xffffffffffffffa7
001f2367: mov    r12d, esi
001f236a: test   esi, esi
001f236c: je     0x1801f23b0
001f236e: nop    
001f2370: cmp    rbx, -0x2d
001f2374: jne    0x1801f2389
001f2376: xor    r8d, r8d
001f2379: mov    rdx, r14
001f237c: mov    rcx, rdi
001f237f: mov    ebx, 1
001f2384: call   0x1801f2060                              ; ʿʳʶʳʾˀʺʸʹʳʽ$$ʺʶʶʾʿʲˁˀʿˁʻ
001f2389: mov    rdx, qword ptr [rdi + 0x40]
001f238d: mov    rcx, qword ptr [rdi + 0x20]
001f2391: movzx  r8d, byte ptr [rdx + rcx]
001f2396: xor    r8b, byte ptr [r14 + rbx]
001f239a: movsxd rcx, r15d
001f239d: inc    r15d
001f23a0: mov    byte ptr [rcx + r13], r8b
001f23a4: inc    qword ptr [rdi + 0x40]
001f23a8: movsxd rcx, r15d
001f23ab: cmp    rcx, r12
001f23ae: jl     0x1801f2370
001f23b0: mov    eax, esi
001f23b2: jmp    0x1801f23b6
001f23b4: xor    eax, eax
001f23b6: lea    rsp, [rbp + 8]
001f23ba: pop    r15
001f23bc: pop    r14
001f23be: pop    r13
001f23c0: pop    r12
001f23c2: pop    rdi
001f23c3: pop    rsi
001f23c4: pop    rbx
001f23c5: pop    rbp
001f23c6: ret    
001f23c7: mov    rbx, qword ptr [rip + 0x3aa92e2]         ; [0x3c9b6b0] metamethod:Method$ʿʳʶʳʾˀʺʸʹʳʽ.ʾʵʿʴʹʺʶʻʶʲʶ()
001f23ce: call   0x182f60790
001f23d3: mov    rdx, rbx
001f23d6: mov    rcx, rax
001f23d9: call   0x182f60c10
001f23de: int3   
001f23df: mov    rbx, qword ptr [rip + 0x3aa92ca]         ; [0x3c9b6b0] metamethod:Method$ʿʳʶʳʾˀʺʸʹʳʽ.ʾʵʿʴʹʺʶʻʶʲʶ()
001f23e6: call   0x182f60790
001f23eb: mov    rdx, rbx
001f23ee: mov    rcx, rax
001f23f1: call   0x182f60c10
001f23f6: int3   
001f23f7: mov    rbx, qword ptr [rip + 0x3aa92b2]         ; [0x3c9b6b0] metamethod:Method$ʿʳʶʳʾˀʺʸʹʳʽ.ʾʵʿʴʹʺʶʻʶʲʶ()
001f23fe: call   0x182f60790
001f2403: mov    rdx, rbx
001f2406: mov    rcx, rax
001f2409: call   0x182f60c10
001f240e: int3   
001f240f: int3   
