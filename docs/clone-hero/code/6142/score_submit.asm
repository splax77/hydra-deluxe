001144c0: mov    qword ptr [rsp + 0x18], rbx
001144c5: mov    qword ptr [rsp + 0x20], rsi
001144ca: mov    qword ptr [rsp + 0x10], rdx
001144cf: mov    qword ptr [rsp + 8], rcx
001144d4: push   rdi
001144d5: push   r14
001144d7: push   r15
001144d9: sub    rsp, 0x90
001144e0: mov    rdi, rdx
001144e3: mov    r14, rcx
001144e6: cmp    byte ptr [rip + 0x3df7ee9], 0            ; [0x3f0c3d6] (bss)
001144ed: jne    0x1801147a6
001144f3: lea    rcx, [rip + 0x3ba6646]                   ; [0x3cbab40] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.Dispose()
001144fa: call   0x182f609b0
001144ff: lea    rcx, [rip + 0x3ba66f2]                   ; [0x3cbabf8] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.MoveNext()
00114506: call   0x182f609b0
0011450b: lea    rcx, [rip + 0x3ba679e]                   ; [0x3cbacb0] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.get_Current()
00114512: call   0x182f609b0
00114517: lea    rcx, [rip + 0x3ba3f92]                   ; [0x3cb84b0] metamethod:Method$System.Collections.Generic.List<string>.GetEnumerator()
0011451e: call   0x182f609b0
00114523: lea    rcx, [rip + 0x3bf44ae]                   ; [0x3d089d8] metamethod:Method$System.MemoryExtensions.AsSpan<ʼʽʶʿˀʵʶʻʴʽʼ>()
0011452a: call   0x182f609b0
0011452f: lea    rcx, [rip + 0x3b96f02]                   ; [0x3cab438] metamethod:Method$System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>.get_Length()
00114536: call   0x182f609b0
0011453b: lea    rcx, [rip + 0x3b82db6]                   ; [0x3c972f8] meta:ʸʽʼʲʲʿʽˁʳˀʽ_TypeInfo
00114542: call   0x182f609b0
00114547: lea    rcx, [rip + 0x3b87362]                   ; [0x3c9b8b0] meta:ʼˁʿʷʶʸʴʼʹʾʶ_TypeInfo
0011454e: call   0x182f609b0
00114553: lea    rcx, [rip + 0x3b8c2a6]                   ; [0x3ca0800] meta:ˁʹʴʸʸʲʸˁʺʸʸ_TypeInfo
0011455a: call   0x182f609b0
0011455f: lea    rcx, [rip + 0x3bd2cfa]                   ; [0x3ce7260] str:'modifiers'
00114566: call   0x182f609b0
0011456b: lea    rcx, [rip + 0x3baf81e]                   ; [0x3cc3d90] str:'stars'
00114572: call   0x182f609b0
00114577: lea    rcx, [rip + 0x3b93162]                   ; [0x3ca76e0] str:'base_score'
0011457e: call   0x182f609b0
00114583: lea    rcx, [rip + 0x3bded7e]                   ; [0x3cf3308] str:'note_score'
0011458a: call   0x182f609b0
0011458f: lea    rcx, [rip + 0x3ba6a62]                   ; [0x3cbaff8] str:'is_pfc'
00114596: call   0x182f609b0
0011459b: lea    rcx, [rip + 0x3bae2ee]                   ; [0x3cc2890] str:'squeezed_score'
001145a2: call   0x182f609b0
001145a7: lea    rcx, [rip + 0x3ba8012]                   ; [0x3cbc5c0] str:'sp_phrases_earned'
001145ae: call   0x182f609b0
001145b3: lea    rcx, [rip + 0x3b849ce]                   ; [0x3c98f88] str:'accent_notes_total'
001145ba: call   0x182f609b0
001145bf: lea    rcx, [rip + 0x3bdf702]                   ; [0x3cf3cc8] str:'notes_hit'
001145c6: call   0x182f609b0
001145cb: lea    rcx, [rip + 0x3b8fd0e]                   ; [0x3ca42e0] str:'audio_calibration'
001145d2: call   0x182f609b0
001145d7: lea    rcx, [rip + 0x3ba820a]                   ; [0x3cbc7e8] str:'sp_ticks_accumulated'
001145de: call   0x182f609b0
001145e3: lea    rcx, [rip + 0x3bda2de]                   ; [0x3cee8c8] str:'no_fail'
001145ea: call   0x182f609b0
001145ef: lea    rcx, [rip + 0x3ba398a]                   ; [0x3cb7f80] str:'solo_bonus_total'
001145f6: call   0x182f609b0
001145fb: lea    rcx, [rip + 0x3b95e9e]                   ; [0x3caa4a0] str:'section_name'
00114602: call   0x182f609b0
00114607: lea    rcx, [rip + 0x3bba4da]                   ; [0x3cceae8] str:'controller_type'
0011460e: call   0x182f609b0
00114613: lea    rcx, [rip + 0x3bdd32e]                   ; [0x3cf1948] str:'end_streak'
0011461a: call   0x182f609b0
0011461f: lea    rcx, [rip + 0x3bc4de2]                   ; [0x3cd9408] str:'time_in_sp'
00114626: call   0x182f609b0
0011462b: lea    rcx, [rip + 0x3bb82be]                   ; [0x3ccc8f0] str:'sustain_score'
00114632: call   0x182f609b0
00114637: lea    rcx, [rip + 0x3ba80f2]                   ; [0x3cbc730] str:'sp_score'
0011463e: call   0x182f609b0
00114643: lea    rcx, [rip + 0x3b95d9e]                   ; [0x3caa3e8] str:'section_count'
0011464a: call   0x182f609b0
0011464f: lea    rcx, [rip + 0x3bf3282]                   ; [0x3d078d8] str:'ghost_notes_total'
00114656: call   0x182f609b0
0011465b: lea    rcx, [rip + 0x3ba8016]                   ; [0x3cbc678] str:'sp_phrases_total'
00114662: call   0x182f609b0
00114667: lea    rcx, [rip + 0x3b9037a]                   ; [0x3ca49e8] str:'avg_multiplier'
0011466e: call   0x182f609b0
00114673: lea    rcx, [rip + 0x3bba7ae]                   ; [0x3ccee28] str:'lefty_flip'
0011467a: call   0x182f609b0
0011467f: lea    rcx, [rip + 0x3bf3192]                   ; [0x3d07818] str:'ghost_notes_hit'
00114686: call   0x182f609b0
0011468b: lea    rcx, [rip + 0x3ba68ae]                   ; [0x3cbaf40] str:'is_fc'
00114692: call   0x182f609b0
00114697: lea    rcx, [rip + 0x3bae132]                   ; [0x3cc27d0] str:'squeezed_notes'
0011469e: call   0x182f609b0
001146a3: lea    rcx, [rip + 0x3b83c2e]                   ; [0x3c982d8] str:'profile_name'
001146aa: call   0x182f609b0
001146af: lea    rcx, [rip + 0x3b95ea2]                   ; [0x3caa558] str:'section_stats'
001146b6: call   0x182f609b0
001146bb: lea    rcx, [rip + 0x3bc9436]                   ; [0x3cddaf8] str:'max_streak'
001146c2: call   0x182f609b0
001146c7: lea    rcx, [rip + 0x3bcc0ea]                   ; [0x3ce07b8] str:'difficulty'
001146ce: call   0x182f609b0
001146d3: lea    rcx, [rip + 0x3b847ee]                   ; [0x3c98ec8] str:'accent_notes_hit'
001146da: call   0x182f609b0
001146df: lea    rcx, [rip + 0x3be0522]                   ; [0x3cf4c08] str:'versus_winner'
001146e6: call   0x182f609b0
001146eb: lea    rcx, [rip + 0x3b8beb6]                   ; [0x3ca05a8] str:'remote_network_player'
001146f2: call   0x182f609b0
001146f7: lea    rcx, [rip + 0x3b93b2a]                   ; [0x3ca8228] str:'score'
001146fe: call   0x182f609b0
00114703: lea    rcx, [rip + 0x3ba677e]                   ; [0x3cbae88] str:'is_bot'
0011470a: call   0x182f609b0
0011470f: lea    rcx, [rip + 0x3bc726a]                   ; [0x3cdb980] str:'total_notes'
00114716: call   0x182f609b0
0011471b: lea    rcx, [rip + 0x3badf2e]                   ; [0x3cc2650] str:'squeeze_note_losses'
00114722: call   0x182f609b0
00114727: lea    rcx, [rip + 0x3bee25a]                   ; [0x3d02988] str:'frets_ghosted'
0011472e: call   0x182f609b0
00114733: lea    rcx, [rip + 0x3bdf4ce]                   ; [0x3cf3c08] str:'notes_count'
0011473a: call   0x182f609b0
0011473f: lea    rcx, [rip + 0x3bb64ca]                   ; [0x3ccac10] str:'combo_score'
00114746: call   0x182f609b0
0011474b: lea    rcx, [rip + 0x3bb3b7e]                   ; [0x3cc82d0] str:'clean_play_bonus'
00114752: call   0x182f609b0
00114757: lea    rcx, [rip + 0x3b9f8d2]                   ; [0x3cb4030] str:'instrument'
0011475e: call   0x182f609b0
00114763: lea    rcx, [rip + 0x3be0c5e]                   ; [0x3cf53c8] str:'excess_hits'
0011476a: call   0x182f609b0
0011476f: lea    rcx, [rip + 0x3be150a]                   ; [0x3cf5c80] str:'video_calibration'
00114776: call   0x182f609b0
0011477b: lea    rcx, [rip + 0x3ba7c16]                   ; [0x3cbc398] str:'sp_bar_ticks'
00114782: call   0x182f609b0
00114787: lea    rcx, [rip + 0x3ba7872]                   ; [0x3cbc000] str:'sp_activations'
0011478e: call   0x182f609b0
00114793: lea    rcx, [rip + 0x3be47d6]                   ; [0x3cf8f70] str:'failed_in_run_time'
0011479a: call   0x182f609b0
0011479f: mov    byte ptr [rip + 0x3df7c30], 1            ; [0x3f0c3d6] (bss)
001147a6: xorps  xmm0, xmm0
001147a9: xor    eax, eax
001147ab: movups xmmword ptr [rsp + 0x50], xmm0
001147b0: mov    qword ptr [rsp + 0x60], rax
001147b5: mov    rax, qword ptr [rip + 0x3b870f4]         ; [0x3c9b8b0] meta:ʼˁʿʷʶʸʴʼʹʾʶ_TypeInfo
001147bc: mov    qword ptr [rsp + 0x20], rax
001147c1: mov    qword ptr [rsp + 0x28], 0xffffffffffffffff
001147ca: movzx  eax, byte ptr [r14 + 0x10]
001147cf: mov    byte ptr [rsp + 0x30], al
001147d3: xor    edx, edx
001147d5: lea    rcx, [rsp + 0x20]
001147da: call   0x181b91030                              ; System.Enum$$ToString
001147df: test   rdi, rdi
001147e2: je     0x180114e20
001147e8: xor    r9d, r9d
001147eb: mov    r8, rax
001147ee: mov    rdx, qword ptr [rip + 0x3b9f83b]         ; [0x3cb4030] str:'instrument'
001147f5: mov    rcx, rdi
001147f8: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
001147fd: mov    rax, qword ptr [rip + 0x3b8bffc]         ; [0x3ca0800] meta:ˁʹʴʸʸʲʸˁʺʸʸ_TypeInfo
00114804: mov    qword ptr [rsp + 0x38], rax
00114809: mov    qword ptr [rsp + 0x40], 0xffffffffffffffff
00114812: movzx  eax, byte ptr [r14 + 0x11]
00114817: mov    byte ptr [rsp + 0x48], al
0011481b: xor    edx, edx
0011481d: lea    rcx, [rsp + 0x38]
00114822: call   0x181b91030                              ; System.Enum$$ToString
00114827: xor    r9d, r9d
0011482a: mov    r8, rax
0011482d: mov    rdx, qword ptr [rip + 0x3bcbf84]         ; [0x3ce07b8] str:'difficulty'
00114834: mov    rcx, rdi
00114837: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
0011483c: xor    r9d, r9d
0011483f: mov    r8, qword ptr [r14 + 0x18]
00114843: mov    rdx, qword ptr [rip + 0x3b83a8e]         ; [0x3c982d8] str:'profile_name'
0011484a: mov    rcx, rdi
0011484d: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
00114852: xor    r8d, r8d
00114855: mov    rdx, qword ptr [rip + 0x3bd2a04]         ; [0x3ce7260] str:'modifiers'
0011485c: mov    rcx, rdi
0011485f: call   0x1823cef70                              ; System.Text.Json.Utf8JsonWriter$$WriteStartArray
00114864: xor    edx, edx
00114866: mov    rcx, r14
00114869: call   0x180116d00                              ; ʹʵʽˁʼʺʷʿʷʲʻ$$ʿʺʾʹʶˁʻʺʲʹʸ
0011486e: test   rax, rax
00114871: je     0x180114e20
00114877: mov    r8, qword ptr [rip + 0x3ba3c32]          ; [0x3cb84b0] metamethod:Method$System.Collections.Generic.List<string>.GetEnumerator()
0011487e: mov    rdx, rax
00114881: lea    rcx, [rsp + 0x70]
00114886: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0011488b: movups xmm0, xmmword ptr [rsp + 0x70]
00114890: movups xmmword ptr [rsp + 0x50], xmm0
00114895: movsd  xmm1, qword ptr [rsp + 0x80]
0011489e: movsd  qword ptr [rsp + 0x60], xmm1
001148a4: xor    esi, esi
001148a6: mov    qword ptr [rsp + 0x20], rsi
001148ab: lea    rbx, [rsp + 0x50]
001148b0: mov    qword ptr [rsp + 0x28], rbx
001148b5: nop    word ptr [rax + rax]
001148c0: mov    rdx, qword ptr [rip + 0x3ba6331]         ; [0x3cbabf8] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.MoveNext()
001148c7: lea    rcx, [rsp + 0x50]
001148cc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
001148d1: test   al, al
001148d3: je     0x1801148e7
001148d5: xor    r8d, r8d
001148d8: mov    rdx, qword ptr [rsp + 0x60]
001148dd: mov    rcx, rdi
001148e0: call   0x1823d49c0                              ; System.Text.Json.Utf8JsonWriter$$WriteStringValue
001148e5: jmp    0x1801148c0
001148e7: mov    rdx, qword ptr [rip + 0x3ba6252]         ; [0x3cbab40] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.Dispose()
001148ee: mov    rcx, rbx
001148f1: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
001148f6: jmp    0x180114929
001148f8: mov    rdx, qword ptr [rip + 0x3ba6241]         ; [0x3cbab40] metamethod:Method$System.Collections.Generic.List.Enumerator<string>.Dispose()
001148ff: mov    rcx, qword ptr [rsp + 0x28]
00114904: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00114909: mov    rcx, qword ptr [rsp + 0x20]
0011490e: test   rcx, rcx
00114911: jne    0x180114e26
00114917: xor    esi, esi
00114919: mov    rdi, qword ptr [rsp + 0xb8]
00114921: mov    r14, qword ptr [rsp + 0xb0]
00114929: xor    edx, edx
0011492b: mov    rcx, rdi
0011492e: call   0x1823c60f0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndArray
00114933: xor    r9d, r9d
00114936: movzx  r8d, byte ptr [r14 + 0x24]
0011493b: mov    rdx, qword ptr [rip + 0x3b8bc66]         ; [0x3ca05a8] str:'remote_network_player'
00114942: mov    rcx, rdi
00114945: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
0011494a: xor    r9d, r9d
0011494d: movzx  r8d, byte ptr [r14 + 0x25]
00114952: mov    rdx, qword ptr [rip + 0x3bba4cf]         ; [0x3ccee28] str:'lefty_flip'
00114959: mov    rcx, rdi
0011495c: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114961: xor    r9d, r9d
00114964: movzx  r8d, byte ptr [r14 + 0x26]
00114969: mov    rdx, qword ptr [rip + 0x3bd9f58]         ; [0x3cee8c8] str:'no_fail'
00114970: mov    rcx, rdi
00114973: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114978: xor    r9d, r9d
0011497b: movss  xmm2, dword ptr [r14 + 0x28]
00114981: mov    rdx, qword ptr [rip + 0x3be12f8]         ; [0x3cf5c80] str:'video_calibration'
00114988: mov    rcx, rdi
0011498b: call   0x1823ccf20                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114990: xor    r9d, r9d
00114993: movss  xmm2, dword ptr [r14 + 0x2c]
00114999: mov    rdx, qword ptr [rip + 0x3b8f940]         ; [0x3ca42e0] str:'audio_calibration'
001149a0: mov    rcx, rdi
001149a3: call   0x1823ccf20                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
001149a8: mov    rax, qword ptr [rip + 0x3b82949]         ; [0x3c972f8] meta:ʸʽʼʲʲʿʽˁʳˀʽ_TypeInfo
001149af: mov    qword ptr [rsp + 0x38], rax
001149b4: mov    qword ptr [rsp + 0x40], 0xffffffffffffffff
001149bd: movzx  eax, byte ptr [r14 + 0x30]
001149c2: mov    byte ptr [rsp + 0x48], al
001149c6: xor    edx, edx
001149c8: lea    rcx, [rsp + 0x38]
001149cd: call   0x181b91030                              ; System.Enum$$ToString
001149d2: xor    r9d, r9d
001149d5: mov    r8, rax
001149d8: mov    rdx, qword ptr [rip + 0x3bba109]         ; [0x3cceae8] str:'controller_type'
001149df: mov    rcx, rdi
001149e2: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
001149e7: xor    r9d, r9d
001149ea: mov    r8d, dword ptr [r14 + 0x34]
001149ee: mov    rdx, qword ptr [rip + 0x3b93833]         ; [0x3ca8228] str:'score'
001149f5: mov    rcx, rdi
001149f8: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
001149fd: xor    r9d, r9d
00114a00: mov    r8d, dword ptr [r14 + 0x38]
00114a04: mov    rdx, qword ptr [rip + 0x3b92cd5]         ; [0x3ca76e0] str:'base_score'
00114a0b: mov    rcx, rdi
00114a0e: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a13: xor    r9d, r9d
00114a16: mov    r8d, dword ptr [r14 + 0x3c]
00114a1a: mov    rdx, qword ptr [rip + 0x3ba355f]         ; [0x3cb7f80] str:'solo_bonus_total'
00114a21: mov    rcx, rdi
00114a24: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a29: xor    r9d, r9d
00114a2c: mov    r8d, dword ptr [r14 + 0x50]
00114a30: mov    rdx, qword ptr [rip + 0x3bade59]         ; [0x3cc2890] str:'squeezed_score'
00114a37: mov    rcx, rdi
00114a3a: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a3f: xor    r9d, r9d
00114a42: mov    r8d, dword ptr [r14 + 0x54]
00114a46: mov    rdx, qword ptr [rip + 0x3bb3883]         ; [0x3cc82d0] str:'clean_play_bonus'
00114a4d: mov    rcx, rdi
00114a50: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a55: xor    r9d, r9d
00114a58: mov    r8d, dword ptr [r14 + 0x40]
00114a5c: mov    rdx, qword ptr [rip + 0x3bde8a5]         ; [0x3cf3308] str:'note_score'
00114a63: mov    rcx, rdi
00114a66: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a6b: xor    r9d, r9d
00114a6e: mov    r8d, dword ptr [r14 + 0x44]
00114a72: mov    rdx, qword ptr [rip + 0x3bb7e77]         ; [0x3ccc8f0] str:'sustain_score'
00114a79: mov    rcx, rdi
00114a7c: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a81: xor    r9d, r9d
00114a84: mov    r8d, dword ptr [r14 + 0x48]
00114a88: mov    rdx, qword ptr [rip + 0x3ba7ca1]         ; [0x3cbc730] str:'sp_score'
00114a8f: mov    rcx, rdi
00114a92: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114a97: xor    r9d, r9d
00114a9a: mov    r8d, dword ptr [r14 + 0x4c]
00114a9e: mov    rdx, qword ptr [rip + 0x3bb616b]         ; [0x3ccac10] str:'combo_score'
00114aa5: mov    rcx, rdi
00114aa8: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114aad: xor    r9d, r9d
00114ab0: mov    r8d, dword ptr [r14 + 0x58]
00114ab4: mov    rdx, qword ptr [rip + 0x3bdf20d]         ; [0x3cf3cc8] str:'notes_hit'
00114abb: mov    rcx, rdi
00114abe: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114ac3: xor    r9d, r9d
00114ac6: mov    r8d, dword ptr [r14 + 0x5c]
00114aca: mov    rdx, qword ptr [rip + 0x3bc6eaf]         ; [0x3cdb980] str:'total_notes'
00114ad1: mov    rcx, rdi
00114ad4: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114ad9: xor    r9d, r9d
00114adc: mov    r8d, dword ptr [r14 + 0x60]
00114ae0: mov    rdx, qword ptr [rip + 0x3bc9011]         ; [0x3cddaf8] str:'max_streak'
00114ae7: mov    rcx, rdi
00114aea: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114aef: xor    r9d, r9d
00114af2: mov    r8d, dword ptr [r14 + 0x64]
00114af6: mov    rdx, qword ptr [rip + 0x3bdce4b]         ; [0x3cf1948] str:'end_streak'
00114afd: mov    rcx, rdi
00114b00: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b05: xor    r9d, r9d
00114b08: mov    r8d, dword ptr [r14 + 0x68]
00114b0c: mov    rdx, qword ptr [rip + 0x3be08b5]         ; [0x3cf53c8] str:'excess_hits'
00114b13: mov    rcx, rdi
00114b16: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b1b: xor    r9d, r9d
00114b1e: mov    r8d, dword ptr [r14 + 0x6c]
00114b22: mov    rdx, qword ptr [rip + 0x3ba7a97]         ; [0x3cbc5c0] str:'sp_phrases_earned'
00114b29: mov    rcx, rdi
00114b2c: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b31: xor    r9d, r9d
00114b34: mov    r8d, dword ptr [r14 + 0x70]
00114b38: mov    rdx, qword ptr [rip + 0x3ba7b39]         ; [0x3cbc678] str:'sp_phrases_total'
00114b3f: mov    rcx, rdi
00114b42: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b47: xor    r9d, r9d
00114b4a: mov    r8d, dword ptr [r14 + 0x74]
00114b4e: mov    rdx, qword ptr [rip + 0x3ba74ab]         ; [0x3cbc000] str:'sp_activations'
00114b55: mov    rcx, rdi
00114b58: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b5d: xor    r9d, r9d
00114b60: movss  xmm2, dword ptr [r14 + 0x78]
00114b66: mov    rdx, qword ptr [rip + 0x3bc489b]         ; [0x3cd9408] str:'time_in_sp'
00114b6d: mov    rcx, rdi
00114b70: call   0x1823ccf20                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b75: xor    r9d, r9d
00114b78: mov    r8, qword ptr [r14 + 0x80]
00114b7f: mov    rdx, qword ptr [rip + 0x3ba7c62]         ; [0x3cbc7e8] str:'sp_ticks_accumulated'
00114b86: mov    rcx, rdi
00114b89: call   0x1823cd0a0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114b8e: xor    r9d, r9d
00114b91: mov    r8, qword ptr [r14 + 0x88]
00114b98: mov    rdx, qword ptr [rip + 0x3ba77f9]         ; [0x3cbc398] str:'sp_bar_ticks'
00114b9f: mov    rcx, rdi
00114ba2: call   0x1823cd0a0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114ba7: xor    r9d, r9d
00114baa: mov    r8d, dword ptr [r14 + 0x90]
00114bb1: mov    rdx, qword ptr [rip + 0x3baf1d8]         ; [0x3cc3d90] str:'stars'
00114bb8: mov    rcx, rdi
00114bbb: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114bc0: xor    r9d, r9d
00114bc3: movss  xmm2, dword ptr [r14 + 0x94]
00114bcc: mov    rdx, qword ptr [rip + 0x3b8fe15]         ; [0x3ca49e8] str:'avg_multiplier'
00114bd3: mov    rcx, rdi
00114bd6: call   0x1823ccf20                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114bdb: xor    r9d, r9d
00114bde: mov    r8d, dword ptr [r14 + 0x98]
00114be5: mov    rdx, qword ptr [rip + 0x3badbe4]         ; [0x3cc27d0] str:'squeezed_notes'
00114bec: mov    rcx, rdi
00114bef: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114bf4: xor    r9d, r9d
00114bf7: mov    r8d, dword ptr [r14 + 0x9c]
00114bfe: mov    rdx, qword ptr [rip + 0x3bada4b]         ; [0x3cc2650] str:'squeeze_note_losses'
00114c05: mov    rcx, rdi
00114c08: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c0d: xor    r9d, r9d
00114c10: mov    r8d, dword ptr [r14 + 0xa0]
00114c17: mov    rdx, qword ptr [rip + 0x3bedd6a]         ; [0x3d02988] str:'frets_ghosted'
00114c1e: mov    rcx, rdi
00114c21: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c26: xor    r9d, r9d
00114c29: mov    r8d, dword ptr [r14 + 0xa4]
00114c30: mov    rdx, qword ptr [rip + 0x3b84291]         ; [0x3c98ec8] str:'accent_notes_hit'
00114c37: mov    rcx, rdi
00114c3a: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c3f: xor    r9d, r9d
00114c42: mov    r8d, dword ptr [r14 + 0xa8]
00114c49: mov    rdx, qword ptr [rip + 0x3b84338]         ; [0x3c98f88] str:'accent_notes_total'
00114c50: mov    rcx, rdi
00114c53: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c58: xor    r9d, r9d
00114c5b: mov    r8d, dword ptr [r14 + 0xac]
00114c62: mov    rdx, qword ptr [rip + 0x3bf2baf]         ; [0x3d07818] str:'ghost_notes_hit'
00114c69: mov    rcx, rdi
00114c6c: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c71: xor    r9d, r9d
00114c74: mov    r8d, dword ptr [r14 + 0xb0]
00114c7b: mov    rdx, qword ptr [rip + 0x3bf2c56]         ; [0x3d078d8] str:'ghost_notes_total'
00114c82: mov    rcx, rdi
00114c85: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114c8a: xor    r9d, r9d
00114c8d: movzx  r8d, byte ptr [r14 + 0xb4]
00114c95: mov    rdx, qword ptr [rip + 0x3ba62a4]         ; [0x3cbaf40] str:'is_fc'
00114c9c: mov    rcx, rdi
00114c9f: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114ca4: xor    r9d, r9d
00114ca7: movzx  r8d, byte ptr [r14 + 0xb5]
00114caf: mov    rdx, qword ptr [rip + 0x3bdff52]         ; [0x3cf4c08] str:'versus_winner'
00114cb6: mov    rcx, rdi
00114cb9: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114cbe: xor    r9d, r9d
00114cc1: movzx  r8d, byte ptr [r14 + 0xb6]
00114cc9: mov    rdx, qword ptr [rip + 0x3ba61b8]         ; [0x3cbae88] str:'is_bot'
00114cd0: mov    rcx, rdi
00114cd3: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114cd8: xor    r9d, r9d
00114cdb: movzx  r8d, byte ptr [r14 + 0xb7]
00114ce3: mov    rdx, qword ptr [rip + 0x3ba630e]         ; [0x3cbaff8] str:'is_pfc'
00114cea: mov    rcx, rdi
00114ced: call   0x1823c5ea0                              ; System.Text.Json.Utf8JsonWriter$$WriteBoolean
00114cf2: xor    r9d, r9d
00114cf5: mov    r8d, dword ptr [r14 + 0xb8]
00114cfc: mov    rdx, qword ptr [rip + 0x3be426d]         ; [0x3cf8f70] str:'failed_in_run_time'
00114d03: mov    rcx, rdi
00114d06: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114d0b: mov    r8, qword ptr [r14 + 0xc0]
00114d12: test   r8, r8
00114d15: je     0x180114e32
00114d1b: xor    r9d, r9d
00114d1e: mov    r8d, dword ptr [r8 + 0x18]
00114d22: mov    rdx, qword ptr [rip + 0x3b956bf]         ; [0x3caa3e8] str:'section_count'
00114d29: mov    rcx, rdi
00114d2c: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114d31: xor    r8d, r8d
00114d34: mov    rdx, qword ptr [rip + 0x3b9581d]         ; [0x3caa558] str:'section_stats'
00114d3b: mov    rcx, rdi
00114d3e: call   0x1823cef70                              ; System.Text.Json.Utf8JsonWriter$$WriteStartArray
00114d43: mov    r15, qword ptr [r14 + 0xc0]
00114d4a: mov    rcx, qword ptr [rip + 0x3bf3c87]         ; [0x3d089d8] metamethod:Method$System.MemoryExtensions.AsSpan<ʼʽʶʿˀʵʶʻʴʽʼ>()
00114d51: cmp    qword ptr [rcx + 0x38], 0
00114d56: jne    0x180114d5d
00114d58: call   0x182f657d0
00114d5d: test   r15, r15
00114d60: je     0x180114d6c
00114d62: lea    r14, [r15 + 0x20]
00114d66: mov    r15d, dword ptr [r15 + 0x18]
00114d6a: jmp    0x180114d83
00114d6c: xorps  xmm0, xmm0
00114d6f: movups xmmword ptr [rsp + 0x20], xmm0
00114d74: psrldq xmm0, 8
00114d79: movd   r15d, xmm0
00114d7e: mov    r14, qword ptr [rsp + 0x20]
00114d83: test   r15d, r15d
00114d86: jle    0x180114dfd
00114d88: nop    dword ptr [rax + rax]
00114d90: cmp    esi, r15d
00114d93: jae    0x180114e2c
00114d99: xor    edx, edx
00114d9b: mov    rcx, rdi
00114d9e: call   0x1823cfaf0                              ; System.Text.Json.Utf8JsonWriter$$WriteStartObject
00114da3: mov    ebx, esi
00114da5: add    rbx, rbx
00114da8: xor    r9d, r9d
00114dab: mov    r8, qword ptr [r14 + rbx*8]
00114daf: mov    rdx, qword ptr [rip + 0x3b956ea]         ; [0x3caa4a0] str:'section_name'
00114db6: mov    rcx, rdi
00114db9: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
00114dbe: xor    r9d, r9d
00114dc1: mov    r8d, dword ptr [r14 + rbx*8 + 0xc]
00114dc6: mov    rdx, qword ptr [rip + 0x3bdeefb]         ; [0x3cf3cc8] str:'notes_hit'
00114dcd: mov    rcx, rdi
00114dd0: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114dd5: xor    r9d, r9d
00114dd8: mov    r8d, dword ptr [r14 + rbx*8 + 8]
00114ddd: mov    rdx, qword ptr [rip + 0x3bdee24]         ; [0x3cf3c08] str:'notes_count'
00114de4: mov    rcx, rdi
00114de7: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00114dec: xor    edx, edx
00114dee: mov    rcx, rdi
00114df1: call   0x1823c63d0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndObject
00114df6: inc    esi
00114df8: cmp    esi, r15d
00114dfb: jl     0x180114d93
00114dfd: xor    edx, edx
00114dff: mov    rcx, rdi
00114e02: call   0x1823c60f0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndArray
00114e07: lea    r11, [rsp + 0x90]
00114e0f: mov    rbx, qword ptr [r11 + 0x30]
00114e13: mov    rsi, qword ptr [r11 + 0x38]
00114e17: mov    rsp, r11
00114e1a: pop    r15
00114e1c: pop    r14
00114e1e: pop    rdi
00114e1f: ret    
00114e20: call   0x182f60c50
00114e25: int3   
00114e26: call   0x182f60ce0
00114e2b: int3   
00114e2c: call   0x182f60c40
00114e31: int3   
00114e32: call   0x182f60c50
00114e37: int3   
00114e38: int3   
