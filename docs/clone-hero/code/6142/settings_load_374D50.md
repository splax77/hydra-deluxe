# settings_load_374D50.asm  (build 6142, RVA 0x374D50, the settings loader, one very large method; the next method starts at 0x377FC0)
Made with chdis.py, 2026-10-30, session b02ab3e6, scout R7. Method start per script.json. Only the parts that matter to timing are explained.
Lines 94-97, 136, 194, 230: first-call string setup that names song_speed, video, audio and the ini section "offsets".
Lines 359-378: build the "audio" setting through 0x210C470: default 0 (r9d is 0, line 364), minimum -2000 (0xFFFFF830 at [rsp+0x20], line 369), maximum 2000 (0x7D0 at [rsp+0x28], line 367), kind 1; store it in the settings statics at +0x18 (line 374).
Lines 379-399: the same for "video" (min -2000, max 2000, default 0) stored at +0x20 (line 394).
Lines 2017-2035: build the "song_speed" setting: default 100 (0x64, line 2020), minimum 10 (0xA, line 2025), maximum 5000 (0x1388, line 2023), kind 5; stored at settings statics +0x10 (line 2030). That object is the one the load coroutine divides by 100.
Everything else (servers, graphics, drums, keys) is outside this scout's slice.
