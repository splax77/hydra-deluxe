# cal_ms_to_s_210C1F0.asm  (build 6142, RVA 0x210C1F0, int setting value / 1000)
Made with chdis.py, 2026-10-30, session b02ab3e6. Method start per script.json.
Lines 10-12: load int at this+0x10, convert to float, divide by 1000.0f (constant at 0x3064904). GameManager.Awake calls it on the audio and the video settings objects (see gm_awake_B41A0.md), so a calibration of 30 becomes 0.030 s.
