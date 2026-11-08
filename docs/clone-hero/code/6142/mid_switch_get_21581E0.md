# mid_switch_get_21581E0 (build 6142, RVA 0x21581E0)
The getter of the .mid reader class #8626's static `ThreadLocal<bool>` (static field +0). Lines 12 to 21: it returns `ThreadLocal<bool>.get_Value` on that field.
The only code that sets the value is in the main .mid reader `0x2155C00` (docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 74 to 76): `set_Value` with the byte at +0x2C of the chart-read settings (class #8267, see `chart_settings_ctor_20E73E0`). A scan of every rip-relative reference to the `set_Value` method slot found only those two lines' function.
