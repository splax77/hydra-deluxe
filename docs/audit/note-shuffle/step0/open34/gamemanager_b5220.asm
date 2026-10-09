000b5220: push   rbx
000b5222: push   rsi
000b5223: sub    rsp, 0x48
000b5227: cmp    byte ptr [rip + 0x3e56f80], 0            ; [0x3f0c1ae] (bss)
000b522e: mov    rbx, rcx
000b5231: jne    0x1800b532e
000b5237: lea    rcx, [rip + 0x3be7a5a]                   ; [0x3c9cc98] meta:BasePlayer[]_TypeInfo
000b523e: call   0x182f609b0
000b5243: lea    rcx, [rip + 0x3c06c0e]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b524a: call   0x182f609b0
000b524f: lea    rcx, [rip + 0x3c0e5c2]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b5256: call   0x182f609b0
000b525b: lea    rcx, [rip + 0x3c27e1e]                   ; [0x3cdd080] meta:DiscordController_TypeInfo
000b5262: call   0x182f609b0
000b5267: lea    rcx, [rip + 0x3beb682]                   ; [0x3ca08f0] meta:UnityEngine.GameObject[]_TypeInfo
000b526e: call   0x182f609b0
000b5273: lea    rcx, [rip + 0x3c0eb76]                   ; [0x3cc3df0] metamethod:Method$UnityEngine.GameObject.GetComponent<BasePlayer>()
000b527a: call   0x182f609b0
000b527f: lea    rcx, [rip + 0x3c108aa]                   ; [0x3cc5b30] metamethod:Method$UnityEngine.GameObject.GetComponent<TrackFadeManager>()
000b5286: call   0x182f609b0
000b528b: lea    rcx, [rip + 0x3c3a5fe]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5292: call   0x182f609b0
000b5297: lea    rcx, [rip + 0x3c29872]                   ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
000b529e: call   0x182f609b0
000b52a3: lea    rcx, [rip + 0x3c2a166]                   ; [0x3cdf410] metamethod:Method$System.Collections.Generic.HashSet<ˁʹʴʸʸʲʸˁʺʸʸ>.Add()
000b52aa: call   0x182f609b0
000b52af: lea    rcx, [rip + 0x3c2991a]                   ; [0x3cdebd0] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Clear()
000b52b6: call   0x182f609b0
000b52bb: lea    rcx, [rip + 0x3c2a20e]                   ; [0x3cdf4d0] metamethod:Method$System.Collections.Generic.HashSet<ˁʹʴʸʸʲʸˁʺʸʸ>.Clear()
000b52c2: call   0x182f609b0
000b52c7: lea    rcx, [rip + 0x3c299c2]                   ; [0x3cdec90] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Contains()
000b52ce: call   0x182f609b0
000b52d3: lea    rcx, [rip + 0x3bfd2ee]                   ; [0x3cb25c8] metamethod:Method$UnityEngine.Object.Instantiate<GameObject>()
000b52da: call   0x182f609b0
000b52df: lea    rcx, [rip + 0x3bfd982]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b52e6: call   0x182f609b0
000b52eb: lea    rcx, [rip + 0x3c19496]                   ; [0x3cce788] metamethod:Method$UnityEngine.Resources.Load<GameObject>()
000b52f2: call   0x182f609b0
000b52f7: lea    rcx, [rip + 0x3c59d4a]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000b52fe: call   0x182f609b0
000b5303: lea    rcx, [rip + 0x3be3016]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b530a: call   0x182f609b0
000b530f: lea    rcx, [rip + 0x3bf6a6a]                   ; [0x3cabd80] meta:ʿʶʴˀʴʾʵʵʶʳʲ[]_TypeInfo
000b5316: call   0x182f609b0
000b531b: lea    rcx, [rip + 0x3be6b56]                   ; [0x3c9be78] str:'Player'
000b5322: call   0x182f609b0
000b5327: mov    byte ptr [rip + 0x3e56e80], 1            ; [0x3f0c1ae] (bss)
000b532e: mov    rcx, qword ptr [rip + 0x3be6b43]         ; [0x3c9be78] str:'Player'
000b5335: xor    edx, edx
000b5337: mov    qword ptr [rsp + 0x60], rbp
000b533c: mov    qword ptr [rsp + 0x68], rdi
000b5341: mov    qword ptr [rsp + 0x70], r12
000b5346: mov    qword ptr [rsp + 0x40], r13
000b534b: mov    qword ptr [rsp + 0x38], r14
000b5350: mov    qword ptr [rsp + 0x30], r15
000b5355: call   0x18287aa10                              ; UnityEngine.GameObject$$FindGameObjectsWithTag
000b535a: mov    rsi, rax
000b535d: test   rax, rax
000b5360: je     0x1800b5c2f
000b5366: xor    edi, edi
000b5368: xor    eax, eax
000b536a: nop    word ptr [rax + rax]
000b5370: cmp    eax, dword ptr [rsi + 0x18]
000b5373: jge    0x1800b53ab
000b5375: cmp    edi, dword ptr [rsi + 0x18]
000b5378: jae    0x1800b5c29
000b537e: mov    rcx, qword ptr [rip + 0x3bfd8e3]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b5385: movsxd rax, edi
000b5388: cmp    dword ptr [rcx + 0xe0], 0
000b538f: mov    rbp, qword ptr [rsi + rax*8 + 0x20]
000b5394: jne    0x1800b539b
000b5396: call   0x182f60cf0
000b539b: xor    edx, edx
000b539d: mov    rcx, rbp
000b53a0: call   0x18287e660                              ; UnityEngine.Object$$Destroy
000b53a5: inc    edi
000b53a7: mov    eax, edi
000b53a9: jmp    0x1800b5370
000b53ab: mov    rax, qword ptr [rip + 0x3c3a4de]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b53b2: cmp    dword ptr [rax + 0xe0], 0
000b53b9: jne    0x1800b53ca
000b53bb: mov    rcx, rax
000b53be: call   0x182f60cf0
000b53c3: mov    rax, qword ptr [rip + 0x3c3a4c6]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b53ca: mov    rax, qword ptr [rax + 0xb8]
000b53d1: mov    rax, qword ptr [rax + 8]
000b53d5: test   rax, rax
000b53d8: je     0x1800b5c2f
000b53de: mov    eax, dword ptr [rax + 0x54]
000b53e1: mov    dword ptr [rbx + 0x190], eax
000b53e7: mov    rax, qword ptr [rip + 0x3c3a4a2]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b53ee: mov    rcx, qword ptr [rax + 0xb8]
000b53f5: mov    rax, qword ptr [rcx + 8]
000b53f9: test   rax, rax
000b53fc: je     0x1800b5c2f
000b5402: cmp    byte ptr [rax + 0x72], 0
000b5406: je     0x1800b5412
000b5408: mov    dword ptr [rbx + 0x190], 1
000b5412: mov    rax, qword ptr [rip + 0x3c0e3ff]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b5419: mov    rcx, qword ptr [rax + 0xb8]
000b5420: mov    rcx, qword ptr [rcx]
000b5423: test   rcx, rcx
000b5426: je     0x1800b5c2f
000b542c: xor    edx, edx
000b542e: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000b5433: test   al, al
000b5435: je     0x1800b548e
000b5437: mov    rax, qword ptr [rip + 0x3c59c0a]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000b543e: mov    rcx, qword ptr [rax + 0xb8]
000b5445: mov    rcx, qword ptr [rcx]
000b5448: test   rcx, rcx
000b544b: je     0x1800b5c2f
000b5451: xor    edx, edx
000b5453: call   0x18008f9e0                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʴˀʺʷʲʹʼʽʽʴʶ
000b5458: test   al, al
000b545a: jne    0x1800b5488
000b545c: mov    rax, qword ptr [rip + 0x3be2ebd]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b5463: xor    edx, edx
000b5465: mov    rcx, qword ptr [rax + 0xb8]
000b546c: mov    rcx, qword ptr [rcx + 0x270]
000b5473: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
000b5478: test   al, al
000b547a: je     0x1800b548e
000b547c: mov    dword ptr [rbx + 0x190], 1
000b5486: jmp    0x1800b548e
000b5488: dec    dword ptr [rbx + 0x190]
000b548e: mov    rcx, qword ptr [rip + 0x3be7803]         ; [0x3c9cc98] meta:BasePlayer[]_TypeInfo
000b5495: mov    edx, 4
000b549a: call   0x182f5fc80
000b549f: lea    rcx, [rbx + 0x58]
000b54a3: mov    qword ptr [rbx + 0x58], rax
000b54a7: mov    rdx, rax
000b54aa: call   0x182f5fc00
000b54af: mov    rcx, qword ptr [rip + 0x3bf68ca]         ; [0x3cabd80] meta:ʿʶʴˀʴʾʵʵʶʳʲ[]_TypeInfo
000b54b6: mov    edx, 4
000b54bb: call   0x182f5fc80
000b54c0: lea    rcx, [rbx + 0x168]
000b54c7: mov    qword ptr [rbx + 0x168], rax
000b54ce: mov    rdx, rax
000b54d1: call   0x182f5fc00
000b54d6: mov    rcx, qword ptr [rbx + 0x108]
000b54dd: mov    dword ptr [rbx + 0xf8], 0
000b54e7: mov    dword ptr [rbx + 0x100], 0
000b54f1: test   rcx, rcx
000b54f4: je     0x1800b5c2f
000b54fa: mov    rdx, qword ptr [rip + 0x3c296cf]         ; [0x3cdebd0] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Clear()
000b5501: call   0x181743720                              ; System.Collections.Generic.HashSet<ˁʶʾʿʲʵʷʲʼʵʻ>$$Clear
000b5506: mov    rcx, qword ptr [rbx + 0x110]
000b550d: test   rcx, rcx
000b5510: je     0x1800b5c2f
000b5516: mov    rdx, qword ptr [rip + 0x3c29fb3]         ; [0x3cdf4d0] metamethod:Method$System.Collections.Generic.HashSet<ˁʹʴʸʸʲʸˁʺʸʸ>.Clear()
000b551d: mov    r13d, 0xffffffff
000b5523: call   0x181743720                              ; System.Collections.Generic.HashSet<ˁʶʾʿʲʵʷʲʼʵʻ>$$Clear
000b5528: mov    rcx, qword ptr [rip + 0x3beb3c1]         ; [0x3ca08f0] meta:UnityEngine.GameObject[]_TypeInfo
000b552f: mov    edx, 4
000b5534: call   0x182f5fc80
000b5539: lea    rcx, [rbx + 0x60]
000b553d: mov    qword ptr [rbx + 0x60], rax
000b5541: mov    rdx, rax
000b5544: call   0x182f5fc00
000b5549: xor    edi, edi
000b554b: nop    dword ptr [rax + rax]
000b5550: mov    rcx, qword ptr [rip + 0x3c3a339]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5557: cmp    dword ptr [rcx + 0xe0], 0
000b555e: jne    0x1800b5565
000b5560: call   0x182f60cf0
000b5565: mov    rax, qword ptr [rip + 0x3c3a324]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b556c: mov    rcx, qword ptr [rax + 0xb8]
000b5573: mov    rsi, qword ptr [rcx + 8]
000b5577: test   rsi, rsi
000b557a: je     0x1800b5c2f
000b5580: mov    rsi, qword ptr [rsi + 0x38]
000b5584: test   rsi, rsi
000b5587: je     0x1800b5c2f
000b558d: cmp    edi, dword ptr [rsi + 0x18]
000b5590: jae    0x1800b5c29
000b5596: mov    rsi, qword ptr [rsi + rdi*8 + 0x20]
000b559b: test   rsi, rsi
000b559e: je     0x1800b5c2f
000b55a4: xor    edx, edx
000b55a6: mov    rcx, rsi
000b55a9: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
000b55ae: test   al, al
000b55b0: je     0x1800b5a7f
000b55b6: mov    rax, qword ptr [rip + 0x3c0e25b]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b55bd: mov    rcx, qword ptr [rax + 0xb8]
000b55c4: mov    rcx, qword ptr [rcx]
000b55c7: test   rcx, rcx
000b55ca: je     0x1800b5c2f
000b55d0: xor    edx, edx
000b55d2: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000b55d7: test   al, al
000b55d9: je     0x1800b560d
000b55db: mov    rax, qword ptr [rip + 0x3be2d3e]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b55e2: xor    edx, edx
000b55e4: mov    rcx, qword ptr [rax + 0xb8]
000b55eb: mov    rcx, qword ptr [rcx + 0x270]
000b55f2: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
000b55f7: test   al, al
000b55f9: je     0x1800b560d
000b55fb: xor    edx, edx
000b55fd: mov    rcx, rsi
000b5600: call   0x180337720                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʿʹʲʸʻʻʼʵʸʵˀ
000b5605: test   al, al
000b5607: jne    0x1800b5a7f
000b560d: xor    r8d, r8d
000b5610: mov    rdx, rsi
000b5613: mov    rcx, rbx
000b5616: call   0x1800ba790                              ; GameManager$$ʹʲʸʾʻʶʴʷˀˀʲ
000b561b: mov    rdx, qword ptr [rip + 0x3c19166]         ; [0x3cce788] metamethod:Method$UnityEngine.Resources.Load<GameObject>()
000b5622: mov    rcx, rax
000b5625: call   0x1804f0800                              ; UnityEngine.Resources$$Load<object>
000b562a: mov    r14, rax
000b562d: test   rax, rax
000b5630: je     0x1800b5c2f
000b5636: xor    r8d, r8d
000b5639: xor    edx, edx
000b563b: mov    rcx, rax
000b563e: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
000b5643: mov    rcx, qword ptr [rip + 0x3bfd61e]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b564a: cmp    dword ptr [rcx + 0xe0], 0
000b5651: jne    0x1800b5658
000b5653: call   0x182f60cf0
000b5658: mov    rdx, qword ptr [rip + 0x3bfcf69]         ; [0x3cb25c8] metamethod:Method$UnityEngine.Object.Instantiate<GameObject>()
000b565f: mov    rcx, r14
000b5662: call   0x1804e6840                              ; UnityEngine.Object$$Instantiate<object>
000b5667: mov    r15, rax
000b566a: mov    rax, qword ptr [rbx + 0x60]
000b566e: test   rax, rax
000b5671: je     0x1800b5c2f
000b5677: cmp    edi, dword ptr [rax + 0x18]
000b567a: jae    0x1800b5c29
000b5680: mov    qword ptr [rax + rdi*8 + 0x20], r15
000b5685: mov    rdx, r15
000b5688: add    rax, 0x20
000b568c: lea    rcx, [rax + rdi*8]
000b5690: call   0x182f5fc00
000b5695: test   r15, r15
000b5698: je     0x1800b5c2f
000b569e: mov    rdx, qword ptr [rip + 0x3c0e74b]         ; [0x3cc3df0] metamethod:Method$UnityEngine.GameObject.GetComponent<BasePlayer>()
000b56a5: mov    rcx, r15
000b56a8: call   0x1804a0650                              ; UnityEngine.GameObject$$GetComponent<object>
000b56ad: mov    r14, rax
000b56b0: test   rax, rax
000b56b3: je     0x1800b5c2f
000b56b9: lea    rcx, [rax + 0x88]
000b56c0: mov    qword ptr [rax + 0x88], rsi
000b56c7: mov    rdx, rsi
000b56ca: call   0x182f5fc00
000b56cf: mov    rdx, qword ptr [rip + 0x3c1045a]         ; [0x3cc5b30] metamethod:Method$UnityEngine.GameObject.GetComponent<TrackFadeManager>()
000b56d6: mov    rcx, r15
000b56d9: call   0x1804a0650                              ; UnityEngine.GameObject$$GetComponent<object>
000b56de: test   rax, rax
000b56e1: je     0x1800b5c2f
000b56e7: lea    rcx, [rax + 0x20]
000b56eb: mov    qword ptr [rax + 0x20], rsi
000b56ef: mov    rdx, rsi
000b56f2: call   0x182f5fc00
000b56f7: xor    r8d, r8d
000b56fa: mov    dl, 1
000b56fc: mov    rcx, r15
000b56ff: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
000b5704: mov    rax, qword ptr [r14]
000b5707: mov    edx, edi
000b5709: mov    rcx, r14
000b570c: mov    r8, qword ptr [rax + 0x420]
000b5713: call   qword ptr [rax + 0x418]
000b5719: mov    r12, qword ptr [rbx + 0x168]
000b5720: test   r12, r12
000b5723: je     0x1800b5c2f
000b5729: mov    r15, qword ptr [r14 + 0x120]
000b5730: test   r15, r15
000b5733: je     0x1800b574e
000b5735: mov    rdx, qword ptr [r12]
000b5739: mov    rcx, r15
000b573c: mov    rdx, qword ptr [rdx + 0x40]
000b5740: call   0x182f5fc20
000b5745: test   rax, rax
000b5748: je     0x1800b5c35
000b574e: cmp    edi, dword ptr [r12 + 0x18]
000b5753: jae    0x1800b5c29
000b5759: lea    rax, [r12 + 0x20]
000b575e: mov    qword ptr [r12 + rdi*8 + 0x20], r15
000b5763: lea    rcx, [rax + rdi*8]
000b5767: mov    rdx, r15
000b576a: call   0x182f5fc00
000b576f: mov    r15, qword ptr [rbx + 0x58]
000b5773: test   r15, r15
000b5776: je     0x1800b5c2f
000b577c: mov    rdx, qword ptr [r15]
000b577f: mov    rcx, r14
000b5782: mov    rdx, qword ptr [rdx + 0x40]
000b5786: call   0x182f5fc20
000b578b: test   rax, rax
000b578e: je     0x1800b5c45
000b5794: cmp    edi, dword ptr [r15 + 0x18]
000b5798: jae    0x1800b5c29
000b579e: lea    rax, [r15 + 0x20]
000b57a2: mov    qword ptr [r15 + rdi*8 + 0x20], r14
000b57a7: lea    rcx, [rax + rdi*8]
000b57ab: mov    rdx, r14
000b57ae: call   0x182f5fc00
000b57b3: mov    rax, qword ptr [rbx + 0x28]
000b57b7: test   rax, rax
000b57ba: je     0x1800b5c2f
000b57c0: cmp    byte ptr [rax + 0x72], 0
000b57c4: jne    0x1800b585a
000b57ca: mov    rax, qword ptr [rip + 0x3be2b4f]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b57d1: xor    edx, edx
000b57d3: mov    rcx, qword ptr [rax + 0xb8]
000b57da: mov    rcx, qword ptr [rcx + 0x10]
000b57de: call   0x182115cd0                              ; ʲʽˀʲʾʿʷʴʼʴʶ$$ʴˁʺʸʲʶʸʵʹʹʿ
000b57e3: cmp    eax, 0x64
000b57e6: jg     0x1800b585a
000b57e8: mov    rax, qword ptr [rip + 0x3be2b31]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b57ef: xor    edx, edx
000b57f1: mov    rcx, qword ptr [rax + 0xb8]
000b57f8: mov    rcx, qword ptr [rcx + 0x90]
000b57ff: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
000b5804: test   al, al
000b5806: je     0x1800b585a
000b5808: mov    rax, qword ptr [rip + 0x3c06649]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b580f: cmp    dword ptr [rax + 0xe0], 0
000b5816: jne    0x1800b5827
000b5818: mov    rcx, rax
000b581b: call   0x182f60cf0
000b5820: mov    rax, qword ptr [rip + 0x3c06631]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b5827: mov    rax, qword ptr [rax + 0xb8]
000b582e: mov    rcx, qword ptr [rax + 8]
000b5832: test   rcx, rcx
000b5835: je     0x1800b5c2f
000b583b: cmp    byte ptr [rcx + 0x3b], 0
000b583f: je     0x1800b585a
000b5841: mov    rax, qword ptr [rsi + 0x10]
000b5845: test   rax, rax
000b5848: je     0x1800b5c2f
000b584e: cmp    byte ptr [rax + 0x10], 9
000b5852: je     0x1800b585a
000b5854: cmp    byte ptr [rax + 0x10], 6
000b5858: jne    0x1800b5885
000b585a: mov    rcx, qword ptr [rbx + 0x58]
000b585e: test   rcx, rcx
000b5861: je     0x1800b5c2f
000b5867: cmp    edi, dword ptr [rcx + 0x18]
000b586a: jae    0x1800b5c29
000b5870: mov    rdx, qword ptr [rcx + rdi*8 + 0x20]
000b5875: test   rdx, rdx
000b5878: je     0x1800b5c2f
000b587e: mov    byte ptr [rdx + 0x80], 0
000b5885: mov    rdx, qword ptr [rsi + 0x10]
000b5889: test   rdx, rdx
000b588c: je     0x1800b5c2f
000b5892: mov    rcx, qword ptr [rbx + 0x108]
000b5899: test   rcx, rcx
000b589c: je     0x1800b5c2f
000b58a2: mov    r8, qword ptr [rip + 0x3c293e7]          ; [0x3cdec90] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Contains()
000b58a9: movzx  edx, byte ptr [rdx + 0x10]
000b58ad: call   0x181743900                              ; System.Collections.Generic.HashSet<SByteEnum>$$Contains
000b58b2: test   al, al
000b58b4: jne    0x1800b58e5
000b58b6: mov    rdx, qword ptr [rsi + 0x10]
000b58ba: test   rdx, rdx
000b58bd: je     0x1800b5c2f
000b58c3: mov    rcx, qword ptr [rbx + 0x108]
000b58ca: test   rcx, rcx
000b58cd: je     0x1800b5c2f
000b58d3: mov    r8, qword ptr [rip + 0x3c29236]          ; [0x3cdeb10] metamethod:Method$System.Collections.Generic.HashSet<ʼˁʿʷʶʸʴʼʹʾʶ>.Add()
000b58da: movzx  edx, byte ptr [rdx + 0x10]
000b58de: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
000b58e3: jmp    0x1800b595d
000b58e5: xor    edx, edx
000b58e7: test   edi, edi
000b58e9: je     0x1800b595d
000b58eb: nop    dword ptr [rax + rax]
000b58f0: mov    rcx, qword ptr [rbx + 0x58]
000b58f4: test   rcx, rcx
000b58f7: je     0x1800b5c2f
000b58fd: cmp    edi, dword ptr [rcx + 0x18]
000b5900: jae    0x1800b5c29
000b5906: mov    rax, qword ptr [rcx + rdi*8 + 0x20]
000b590b: test   rax, rax
000b590e: je     0x1800b5c2f
000b5914: mov    rax, qword ptr [rax + 0x88]
000b591b: test   rax, rax
000b591e: je     0x1800b5c2f
000b5924: mov    rcx, qword ptr [rax + 0x10]
000b5928: test   rcx, rcx
000b592b: je     0x1800b5c2f
000b5931: mov    rax, qword ptr [rsi + 0x10]
000b5935: test   rax, rax
000b5938: je     0x1800b5c2f
000b593e: movzx  eax, byte ptr [rax + 0x10]
000b5942: cmp    byte ptr [rcx + 0x10], al
000b5945: jne    0x1800b5957
000b5947: mov    rax, qword ptr [rbx + 0x58]
000b594b: mov    rcx, qword ptr [rax + rdi*8 + 0x20]
000b5950: mov    byte ptr [rcx + 0x80], 0
000b5957: inc    edx
000b5959: cmp    edx, edi
000b595b: jl     0x1800b58f0
000b595d: mov    rcx, qword ptr [rbx + 0x58]
000b5961: test   rcx, rcx
000b5964: je     0x1800b5c2f
000b596a: cmp    edi, dword ptr [rcx + 0x18]
000b596d: jae    0x1800b5c29
000b5973: mov    rdx, qword ptr [rcx + rdi*8 + 0x20]
000b5978: test   rdx, rdx
000b597b: je     0x1800b5c2f
000b5981: cmp    byte ptr [rdx + 0x80], 0
000b5988: je     0x1800b59d6
000b598a: mov    rax, qword ptr [rip + 0x3c064c7]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b5991: cmp    dword ptr [rax + 0xe0], 0
000b5998: jne    0x1800b59a9
000b599a: mov    rcx, rax
000b599d: call   0x182f60cf0
000b59a2: mov    rax, qword ptr [rip + 0x3c064af]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b59a9: mov    rdx, qword ptr [rsi + 0x10]
000b59ad: test   rdx, rdx
000b59b0: je     0x1800b5c2f
000b59b6: mov    rax, qword ptr [rax + 0xb8]
000b59bd: mov    rcx, qword ptr [rax + 8]
000b59c1: test   rcx, rcx
000b59c4: je     0x1800b5c2f
000b59ca: movzx  edx, byte ptr [rdx + 0x10]
000b59ce: xor    r8d, r8d
000b59d1: call   0x180365a50                              ; BassAudioManager$$ʴʲʹʲʼʻʲʿʷʶʴ
000b59d6: mov    rdx, qword ptr [rsi + 0x10]
000b59da: cmp    r13d, -1
000b59de: mov    eax, edi
000b59e0: cmovne eax, r13d
000b59e4: mov    r13d, eax
000b59e7: test   rdx, rdx
000b59ea: je     0x1800b5c2f
000b59f0: mov    rcx, qword ptr [rbx + 0x110]
000b59f7: test   rcx, rcx
000b59fa: je     0x1800b5c2f
000b5a00: mov    r8, qword ptr [rip + 0x3c29a09]          ; [0x3cdf410] metamethod:Method$System.Collections.Generic.HashSet<ˁʹʴʸʸʲʸˁʺʸʸ>.Add()
000b5a07: movzx  edx, byte ptr [rdx + 0x11]
000b5a0b: call   0x181743660                              ; System.Collections.Generic.HashSet<SByteEnum>$$Add
000b5a10: xor    edx, edx
000b5a12: mov    rcx, rsi
000b5a15: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
000b5a1a: test   al, al
000b5a1c: jne    0x1800b5a26
000b5a1e: inc    dword ptr [rbx + 0x100]
000b5a24: jmp    0x1800b5a2c
000b5a26: inc    dword ptr [rbx + 0xf8]
000b5a2c: cmp    byte ptr [rbx + 0xfc], 0
000b5a33: je     0x1800b5a49
000b5a35: xor    edx, edx
000b5a37: mov    rcx, rsi
000b5a3a: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
000b5a3f: test   al, al
000b5a41: jne    0x1800b5a49
000b5a43: mov    byte ptr [rbx + 0xfc], al
000b5a49: mov    rcx, qword ptr [rip + 0x3c39e40]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5a50: cmp    dword ptr [rcx + 0xe0], 0
000b5a57: jne    0x1800b5a5e
000b5a59: call   0x182f60cf0
000b5a5e: mov    rax, qword ptr [rip + 0x3c39e2b]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5a65: mov    rcx, qword ptr [rax + 0xb8]
000b5a6c: mov    rax, qword ptr [rcx + 8]
000b5a70: test   rax, rax
000b5a73: je     0x1800b5c2f
000b5a79: cmp    byte ptr [rax + 0x72], 0
000b5a7d: jne    0x1800b5a8a
000b5a7f: inc    edi
000b5a81: cmp    edi, 4
000b5a84: jl     0x1800b5550
000b5a8a: mov    rcx, qword ptr [rbx + 0x40]
000b5a8e: test   rcx, rcx
000b5a91: je     0x1800b5c2f
000b5a97: mov    rdx, qword ptr [rbx + 0x168]
000b5a9e: xor    r8d, r8d
000b5aa1: call   0x1802e00c0                              ; StarProgress$$ʽʿʿʻʽʲʶʵʶʹʲ
000b5aa6: mov    rdx, qword ptr [rbx + 0x60]
000b5aaa: xor    r8d, r8d
000b5aad: mov    rcx, rbx
000b5ab0: call   0x1800b84e0                              ; GameManager$$ʷʳʴʹˁʽˁʳʾʹʴ
000b5ab5: mov    rax, qword ptr [rip + 0x3c275c4]         ; [0x3cdd080] meta:DiscordController_TypeInfo
000b5abc: mov    rcx, qword ptr [rax + 0xb8]
000b5ac3: mov    rdi, qword ptr [rcx]
000b5ac6: mov    rcx, qword ptr [rip + 0x3c39dc3]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5acd: cmp    dword ptr [rcx + 0xe0], 0
000b5ad4: jne    0x1800b5ae2
000b5ad6: call   0x182f60cf0
000b5adb: mov    rcx, qword ptr [rip + 0x3c39dae]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5ae2: mov    rax, qword ptr [rcx + 0xb8]
000b5ae9: mov    rdx, qword ptr [rax + 8]
000b5aed: test   rdx, rdx
000b5af0: je     0x1800b5c2f
000b5af6: mov    r10, qword ptr [rdx + 0x38]
000b5afa: test   r10, r10
000b5afd: je     0x1800b5c2f
000b5b03: cmp    r13d, dword ptr [r10 + 0x18]
000b5b07: jae    0x1800b5c29
000b5b0d: test   rdi, rdi
000b5b10: je     0x1800b5c2f
000b5b16: cmp    dword ptr [rdx + 0x54], 1
000b5b1a: mov    rcx, rdi
000b5b1d: mov    r9, qword ptr [rdx + 0x28]
000b5b21: movzx  edx, byte ptr [rdx + 0x72]
000b5b25: setg   r8b
000b5b29: movsxd rax, r13d
000b5b2c: mov    qword ptr [rsp + 0x28], 0
000b5b35: mov    rax, qword ptr [r10 + rax*8 + 0x20]
000b5b3a: mov    qword ptr [rsp + 0x20], rax
000b5b3f: call   0x180199700                              ; DiscordController$$ʼʹʿˀʵʴʴʽʴʵʴ
000b5b44: mov    rax, qword ptr [rip + 0x3c39d45]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5b4b: mov    rcx, qword ptr [rax + 0xb8]
000b5b52: mov    rax, qword ptr [rcx + 8]
000b5b56: test   rax, rax
000b5b59: je     0x1800b5c2f
000b5b5f: cmp    byte ptr [rax + 0x72], 0
000b5b63: je     0x1800b5bfd
000b5b69: mov    rdx, qword ptr [rbx + 0x58]
000b5b6d: xor    edi, edi
000b5b6f: xor    eax, eax
000b5b71: test   rdx, rdx
000b5b74: je     0x1800b5c2f
000b5b7a: nop    word ptr [rax + rax]
000b5b80: cmp    eax, dword ptr [rdx + 0x18]
000b5b83: jge    0x1800b5bfd
000b5b85: mov    rcx, qword ptr [rbx + 0x58]
000b5b89: test   rcx, rcx
000b5b8c: je     0x1800b5c2f
000b5b92: cmp    edi, dword ptr [rcx + 0x18]
000b5b95: jae    0x1800b5c29
000b5b9b: movsxd rax, edi
000b5b9e: mov    rsi, qword ptr [rcx + rax*8 + 0x20]
000b5ba3: mov    rcx, qword ptr [rip + 0x3bfd0be]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b5baa: cmp    dword ptr [rcx + 0xe0], 0
000b5bb1: jne    0x1800b5bb8
000b5bb3: call   0x182f60cf0
000b5bb8: xor    r8d, r8d
000b5bbb: xor    edx, edx
000b5bbd: mov    rcx, rsi
000b5bc0: call   0x18287fed0                              ; UnityEngine.Object$$op_Inequality
000b5bc5: mov    rdx, qword ptr [rbx + 0x58]
000b5bc9: test   al, al
000b5bcb: jne    0x1800b5bd8
000b5bcd: inc    edi
000b5bcf: mov    eax, edi
000b5bd1: test   rdx, rdx
000b5bd4: je     0x1800b5c2f
000b5bd6: jmp    0x1800b5b80
000b5bd8: test   rdx, rdx
000b5bdb: je     0x1800b5c2f
000b5bdd: cmp    edi, dword ptr [rdx + 0x18]
000b5be0: jae    0x1800b5c29
000b5be2: movsxd rax, edi
000b5be5: lea    rcx, [rbx + 0x160]
000b5bec: mov    rdx, qword ptr [rdx + rax*8 + 0x20]
000b5bf1: mov    qword ptr [rbx + 0x160], rdx
000b5bf8: call   0x182f5fc00
000b5bfd: mov    r15, qword ptr [rsp + 0x30]
000b5c02: mov    r14, qword ptr [rsp + 0x38]
000b5c07: mov    r13, qword ptr [rsp + 0x40]
000b5c0c: mov    r12, qword ptr [rsp + 0x70]
000b5c11: mov    rdi, qword ptr [rsp + 0x68]
000b5c16: mov    rbp, qword ptr [rsp + 0x60]
000b5c1b: mov    byte ptr [rbx + 0x159], 1
000b5c22: add    rsp, 0x48
000b5c26: pop    rsi
000b5c27: pop    rbx
000b5c28: ret    
000b5c29: call   0x182f60c40
000b5c2e: int3   
000b5c2f: call   0x182f60c50
000b5c34: int3   
000b5c35: call   0x182f604f0
000b5c3a: mov    rcx, rax
000b5c3d: xor    edx, edx
000b5c3f: call   0x182f60c10
000b5c44: int3   
000b5c45: call   0x182f604f0
000b5c4a: mov    rcx, rax
000b5c4d: xor    edx, edx
000b5c4f: call   0x182f60c10
000b5c54: int3   
000b5c55: int3   
