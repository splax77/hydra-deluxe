"""Reverse-engineering facts for the Clone Hero drum hit-window probe.

Every address here is transcribed from the handoff spec
(docs/superpowers/specs/2026-09-17-ch-dynamic-input-probe.md), which in turn
cross-checked them against the Ghidra dumps in ../hydra-data/scratch_ms/. This is the ONE
place the numbers live. Every other module imports from here; nothing else
hard-codes an address. If a Clone Hero update shifts the binary, this file is
the only thing that changes.

Two kinds of address appear in the dumps:

  * RVA  -- an offset from the GameAssembly.dll load base. The functions below
           are given as RVAs directly in the spec.
  * Ghidra VA -- an absolute address Ghidra printed, e.g. DAT_1831406c8. Ghidra
           loaded the module at GHIDRA_IMAGE_BASE (0x180000000), so
           RVA = VA - 0x180000000. We store the resolved RVAs below so callers
           never have to redo that subtraction.

To turn any RVA into a live address at run time: live = module_base + rva,
where module_base is where GameAssembly.dll actually loaded in the running
process (found by the process/address layer).

Build these were captured from: Clone Hero v1.1.0.6142, Unity IL2CPP x64,
engine "StrikeCore".
"""

# The base Ghidra assumed when it printed absolute VAs. Used only to document
# how the DAT RVAs below were derived; the live base is discovered at run time.
GHIDRA_IMAGE_BASE = 0x180000000


# --- Code: function RVAs (offsets from the GameAssembly.dll base) ------------
#
# These are breakpoint targets. Names match the spec's labels.

# double FUN_1820ddda0(longlong self, double t) -- the dynamic window formula.
# No clamp inside it; both branches return the raw quotient. The result comes
# back in xmm0. Selected branch depends on the PrecisionMode bit (see FLAGS).
RVA_WINDOW_FORMULA = 0x20DDDA0

# DrumsEngine constructor. On entry the engine object is in rcx (x64 fastcall /
# IL2CPP). Best place to capture the live object pointer. Writes the three
# window fields (offsets 0x20/0x30/0x38 below).
RVA_DRUMS_ENGINE_CTOR = 0x20DF680

# Shared base-engine setup called by the constructor.
RVA_BASE_ENGINE_CTOR_DRUMS = 0x100E960

# Per-note hit loop: where a hit is matched to a note. Correlate a fired input
# with the note and delta the engine assigned it here.
RVA_NOTE_PROCESSING = 0x20DCC90

# Hit-decision comparison against the window field. WARNING: the decompiler
# indexes the object as longlong* (word-indexed) in this routine, so an index
# of 0x20 there is BYTE offset 0x100. Confirm every offset here live.
RVA_HIT_CHECK = 0x100DAB0


# --- Data: engine-object byte offsets (from the object pointer) --------------
#
# All doubles unless noted. From the constructor, where the casts are
# unambiguous. Still confirm once live.

# Total search window (double). Set to back*2 at construction, then overwritten
# per note by the formula's caller. THIS is the field the passive probe watches.
OFF_TOTAL_WINDOW = 0x20

# Back-window constant (double). Max per-side, ~85 ms normal.
OFF_BACK_WINDOW = 0x30

# Front-window constant (double). Min per-side, ~37.5 ms normal.
OFF_FRONT_WINDOW = 0x38

# Song clock (double, seconds). Proven live on 2026-09-25: play_chart.py reads
# it continuously and plays whole songs on time. The Ghidra notes called this
# field "hit time"; the running game shows it is the song clock.
OFF_SONG_CLOCK = 0x100

# Score (u32). Rises only when a note is hit (proven by play_chart.py).
OFF_SCORE = 0x94

# Hit-time candidate (double, seconds). The code reading says the game copies
# the song clock here on a hit. NOT yet confirmed live: step 4 of
# docs/superpowers/plans/2026-09-25-hit-window-testing.md checks it.
OFF_HIT_TIME = 0x2E0

# Flags dword. Bit 0x1000 is PrecisionMode (clear = normal).
OFF_FLAGS = 0x198
PRECISION_MODE_BIT = 0x1000

# Note count used by the processing loop (dword).
OFF_NOTE_COUNT = 0x8C


# --- Data: .rdata constant globals, as RVAs ----------------------------------
#
# Doubles in .rdata. Read them live once the module base is known. Each entry
# is VA - GHIDRA_IMAGE_BASE so it can be added straight to module_base.

# Per-side window constants.  Stored as SECONDS in the binary (not ms).
# Labels were swapped in the original Ghidra analysis; corrected 2026-09-25
# after live verification: 0x31406C8 holds 0.0375 s (front) and
# 0x31406E0 holds 0.085 s (back).
RVA_CONST_NORMAL_BACK = 0x31406E0      # DAT_1831406e0, expect 0.085 s (= 85 ms)
RVA_CONST_NORMAL_FRONT = 0x31406C8     # DAT_1831406c8, expect 0.0375 s (= 37.5 ms)
RVA_CONST_PRECISION_BACK = 0x31406D0   # DAT_1831406d0, expect 0.040 s (= 40 ms)
RVA_CONST_PRECISION_FRONT = 0x31406B8  # DAT_1831406b8, expect 0.025 s (= 25 ms)

# Formula constants, normal branch: window = (t*C1 - pow(t,e)*C2)*C3 - C4, all
# over the divisor. (Names are positional; confirm decimals live.)
RVA_FORMULA_NORMAL = {
    "c1": 0x3140738,   # _DAT_183140738
    "c2": 0x3140658,   # _DAT_183140658
    "c3": 0x31406F8,   # _DAT_1831406f8
    "c4": 0x3140700,   # DAT_183140700
}

# Formula constants, precision branch: window = (C0 - (t*C1 - pow(t,e)*C2)*C3)
# over the divisor.
RVA_FORMULA_PRECISION = {
    "c1": 0x31406B0,   # _DAT_1831406b0
    "c2": 0x3140620,   # _DAT_183140620
    "c3": 0x3140708,   # _DAT_183140708
    "c0": 0x3063FC0,   # DAT_183063fc0
}

# Shared divisor C5 (both branches). t is pre-scaled by C5 and the result is
# divided by it; probably 1000 for a seconds<->ms conversion.
RVA_FORMULA_DIVISOR = 0x3064C28        # DAT_183064c28, expect ~1000

# Exponent e in pow(t, e). The spec warns NOT to assume e == 2; read it live.
RVA_FORMULA_EXPONENT = 0x30658A8       # DAT_1830658a8

# A threshold used in the hit-check comparison; capture it, it may be part of
# the enforced-window logic.
RVA_HITCHECK_THRESHOLD = 0x31406E8     # DAT_1831406e8


# --- Sanity-check expectations -----------------------------------------------
#
# Milestone 1 in the spec: read the two normal constants and print them. If
# they come out at these values (within tolerance), the whole address pipeline
# is correct and everything downstream can be trusted.

# --- Units ---------------------------------------------------------------------
#
# The game stores times in seconds; the probe reports ms. Every conversion
# between the two goes through these helpers.
MS_PER_S = 1000.0


def s_to_ms(s: float) -> float:
    """Seconds (the game's unit) to ms (the probe's)."""
    return s * MS_PER_S


def ms_to_s(ms: float) -> float:
    """Ms (the probe's unit) to seconds (the game's)."""
    return ms / MS_PER_S


# Expected values in SECONDS (the game's native unit for these constants).
EXPECT_NORMAL_BACK_S = 0.085
EXPECT_NORMAL_FRONT_S = 0.0375
EXPECT_PRECISION_BACK_S = 0.040

# The back-window edges in ms, which the passive probe's clamp verdict uses.
EXPECT_NORMAL_BACK_MS = s_to_ms(EXPECT_NORMAL_BACK_S)
EXPECT_PRECISION_BACK_MS = s_to_ms(EXPECT_PRECISION_BACK_S)

# Tolerance (ms) for calling a live-read constant "the value we expected".
CONST_MATCH_TOLERANCE_MS = 0.5

# --- Measured whole-window cap and floor (normal mode) ------------------------
#
# Clone Hero 1.1.0.6142 sets each note's window from both neighbouring gaps.
# 0x20F7210 halves each gap and holds the half between the front and back
# constants (37.5 and 85 ms). The two halves are summed and passed to the
# window formula (RVA_WINDOW_FORMULA, 0x20DDDA0). So the formula's input runs
# from 75 to 170 ms, and its output from 96.2932 to 171.4313 ms: the cap is a
# clamp on the input, not on the stored window. The formula with the DLL's
# constants gives 171.4313075 at 170 and 96.2931672 at 75, and
# experiments/results/poll_windows.csv agrees: it tops out at 171.431308 ms
# (6 rows) and bottoms out at 96.293167 ms (22 rows). Evidence:
# docs/audit/ch-evidence.md, section 77.
# Normal mode only; nobody has read precision mode's cap.
WINDOW_CAP_MS = 171.4313
WINDOW_FLOOR_MS = 96.2932

# Half the whole window: the per-side reach at the widest spacing (85.72 ms,
# not the 85 ms back constant).
ONE_SIDE_CAP_MS = WINDOW_CAP_MS / 2

# A note reads the cap when the gaps on both sides are at least this wide
# (each half-gap at the back constant), and the floor when both are at most
# this narrow (each half-gap at the front constant).
CAP_FROM_GAP_MS = 2 * EXPECT_NORMAL_BACK_MS               # 170
FLOOR_UP_TO_GAP_MS = 2 * s_to_ms(EXPECT_NORMAL_FRONT_S)   # 75

# How close a stored reading must be to count as the cap or the floor. The
# measured values agree with these to 0.00001 ms.
WINDOW_MATCH_TOLERANCE_MS = 0.01

# How far a stored window (or hit time) must move between two reads to count
# as a change rather than the same value read again. The game writes these
# doubles whole, so anything above rounding noise is a real change.
# experiments/live.window_changed is the one place that compares against it.
WINDOW_CHANGE_TOLERANCE_MS = 1e-6

# Name of the target module and process.
MODULE_NAME = "GameAssembly.dll"
PROCESS_NAME = "Clone Hero.exe"
# The game's window title, for the runners that find its window to send keys.
WINDOW_TITLE = "Clone Hero"

# ---- keys of EngineModel.constants() ------------------------------------
# engine.py writes these and experiments/analysis.py reads them, so both
# sides spell each key from here. The four window keys are built from their
# mode prefix, so renaming a prefix carries to them.
CONST_KEY_DIVISOR = "divisor"
CONST_KEY_EXPONENT = "exponent"
CONST_KEY_PREFIX_NORMAL = "normal_"
CONST_KEY_PREFIX_PRECISION = "precision_"
CONST_KEY_NORMAL_BACK = CONST_KEY_PREFIX_NORMAL + "back"
CONST_KEY_NORMAL_FRONT = CONST_KEY_PREFIX_NORMAL + "front"
CONST_KEY_PRECISION_BACK = CONST_KEY_PREFIX_PRECISION + "back"
CONST_KEY_PRECISION_FRONT = CONST_KEY_PREFIX_PRECISION + "front"
CONST_KEY_HITCHECK_THRESHOLD = "hitcheck_threshold"

# ---- probe cut-offs (D54: recorded as they are) ---------------------------
# Each value is the one the probe has always used; D54 records them without
# a new measurement. The comment names the verdict and the file that reads it.

# engine_finder.find_live_engine: a candidate whose total window is below
# this is not a live engine (an empty or freed object).
ENGINE_EMPTY_WINDOW_S = 0.001
# engine_finder.find_live_engine: a clock that moved more than this between
# its two reads belongs to the song that is playing.
ENGINE_CLOCK_MOVED_S = 1e-6
# experiments/watch_window.hit_time_report: the furthest a stored hit time may
# sit from its nearest note for +0x2e0 to look like the hit time.
HIT_TIME_MAX_GAP_MS = 10.0
# experiments/walk_edges.SongClock: a clock change seen within this of the
# previous read is fresh, so the time since it can be filled in.
CLOCK_FRESH_S = 0.002
# experiments/walk_edges.SongClock: the most time it fills in past a fresh
# change, so a pause cannot run the estimate away.
CLOCK_MAX_FILL_S = 0.05
# experiments/analysis.clamp_verdict: how close two window values must be to
# count as equal.
CLAMP_TOLERANCE_MS = 1.0
# experiments/analysis.clamp_verdict: the share of above-cap notes that must
# agree before the verdict is called.
CLAMP_DECISIVE_FRACTION = 0.8

# The runners read a press's result this long after its note (D54).
INPUT_SETTLE_MS = 250

# ---- probe chart layout -------------------------------------------------
# Note-pair spacings the probe chart lays out, and the .chart NOTE number it
# writes (0 is the kick in a .chart). This is a chart note, not an input lane:
# input lanes are input_driver.Lane, where the kick is Lane.KICK (4).
# probe_chart.py and experiments/active_probe.py both read these.
PROBE_SPACINGS_MS: tuple[float, ...] = (
    30, 50, 100, 150, 180, 185, 190, 195, 205, 211, 220, 240, 300,
)
PROBE_CHART_NOTE_KICK = 0
