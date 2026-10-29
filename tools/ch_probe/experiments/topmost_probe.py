"""Show a plain always-on-top window for one minute, to see whether a window
like that stays visible over Clone Hero.

Plain version: an overlay is only useful if Windows draws it above the game.
This script opens the simplest such window: a half-see-through black strip at
the top centre of the main screen with a white label "HYDRA OVERLAY PROBE
hh:mm:ss" that ticks once a second. Clicks pass straight through it to the
game, and it never takes the keyboard focus. It closes itself after
LIFETIME_S seconds. The user's eyes are the check: the write-up records
"visible" or "not visible" for each fullscreen setting.

It never opens, reads or sends input to the game.

Usage (from the repo root, only at the sitting):
    py tools\\ch_probe\\experiments\\topmost_probe.py
"""

from __future__ import annotations

import sys
import time
from typing import Optional, TextIO

# The window's styles. WS_EX_TOPMOST keeps it above every normal window,
# WS_EX_LAYERED lets it be half see-through, and WS_EX_TRANSPARENT passes
# mouse clicks through to whatever is below. WS_POPUP gives it no frame or
# title bar.
WS_EX_TOPMOST = 0x00000008
WS_EX_LAYERED = 0x00080000
WS_EX_TRANSPARENT = 0x00000020
WS_POPUP = 0x80000000
EX_STYLE = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT

# Size and lifetime (the plan's Task D step 2).
WINDOW_W = 600
WINDOW_H = 120
LIFETIME_S = 60

# "Half-transparent": the layered window's alpha, out of 255.
ALPHA = 128
# The label's font height in pixels. The whole label has to fit inside
# WINDOW_W, because the text is clipped at the window's edge: Segoe UI bold at
# 48 px measures 765 px wide, at 32 px 509 px (GetTextExtentPoint32W on the
# longest label, "HYDRA OVERLAY PROBE 00:00:00").
FONT_PX = 32
# The label repaints this often, so its clock ticks each second.
REPAINT_MS = 1000

LABEL_PREFIX = "HYDRA OVERLAY PROBE "

# Win32 message, flag and stock-object numbers the window uses.
_WM_DESTROY = 0x0002
_WM_PAINT = 0x000F
_WM_TIMER = 0x0113
_GWL_EXSTYLE = -20
_SM_CXSCREEN = 0
_SW_SHOWNOACTIVATE = 4
_LWA_ALPHA = 0x2
_BLACK_BRUSH = 4
_TRANSPARENT_BK = 1
_WHITE = 0x00FFFFFF
_FW_BOLD = 700
_DEFAULT_CHARSET = 1
_DT_CENTER = 0x1
_DT_VCENTER = 0x4
_DT_SINGLELINE = 0x20
_TIMER_ID = 1
_CLASS_NAME = "HydraOverlayProbe"


def label_text(moment: Optional[time.struct_time] = None) -> str:
    """The label the window shows: the fixed words and the local time."""
    return LABEL_PREFIX + time.strftime("%H:%M:%S", moment or time.localtime())


def run_probe(out: Optional[TextIO] = None) -> int:
    """Open the topmost window, keep it up for LIFETIME_S seconds, then close
    it. Prints the window handle and the extended style Windows reports for
    it. Returns 0, or 1 when Windows refuses to make the window or its timer.

    LIVE-ONLY: this puts a real window on the user's screen. It runs only at
    the sitting, never in a test."""
    import ctypes
    from ctypes import wintypes

    out = sys.stdout if out is None else out
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    gdi32 = ctypes.WinDLL("gdi32", use_last_error=True)
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

    LRESULT = ctypes.c_ssize_t
    WNDPROC = ctypes.WINFUNCTYPE(
        LRESULT, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)

    class WNDCLASSW(ctypes.Structure):
        _fields_ = [
            ("style", wintypes.UINT),
            ("lpfnWndProc", WNDPROC),
            ("cbClsExtra", ctypes.c_int),
            ("cbWndExtra", ctypes.c_int),
            ("hInstance", wintypes.HINSTANCE),
            ("hIcon", wintypes.HICON),
            ("hCursor", wintypes.HANDLE),
            ("hbrBackground", wintypes.HBRUSH),
            ("lpszMenuName", wintypes.LPCWSTR),
            ("lpszClassName", wintypes.LPCWSTR),
        ]

    class PAINTSTRUCT(ctypes.Structure):
        _fields_ = [
            ("hdc", wintypes.HDC),
            ("fErase", wintypes.BOOL),
            ("rcPaint", wintypes.RECT),
            ("fRestore", wintypes.BOOL),
            ("fIncUpdate", wintypes.BOOL),
            ("rgbReserved", wintypes.BYTE * 32),
        ]

    kernel32.GetModuleHandleW.argtypes = [wintypes.LPCWSTR]
    kernel32.GetModuleHandleW.restype = wintypes.HMODULE
    user32.DefWindowProcW.argtypes = [
        wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
    user32.DefWindowProcW.restype = LRESULT
    user32.RegisterClassW.argtypes = [ctypes.POINTER(WNDCLASSW)]
    user32.RegisterClassW.restype = wintypes.ATOM
    user32.UnregisterClassW.argtypes = [wintypes.LPCWSTR, wintypes.HINSTANCE]
    user32.UnregisterClassW.restype = wintypes.BOOL
    user32.CreateWindowExW.argtypes = [
        wintypes.DWORD, wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.DWORD,
        ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
        wintypes.HWND, wintypes.HMENU, wintypes.HINSTANCE, wintypes.LPVOID]
    user32.CreateWindowExW.restype = wintypes.HWND
    user32.SetLayeredWindowAttributes.argtypes = [
        wintypes.HWND, wintypes.COLORREF, wintypes.BYTE, wintypes.DWORD]
    user32.SetLayeredWindowAttributes.restype = wintypes.BOOL
    user32.GetSystemMetrics.argtypes = [ctypes.c_int]
    user32.GetSystemMetrics.restype = ctypes.c_int
    user32.ShowWindow.argtypes = [wintypes.HWND, ctypes.c_int]
    user32.ShowWindow.restype = wintypes.BOOL
    user32.UpdateWindow.argtypes = [wintypes.HWND]
    user32.UpdateWindow.restype = wintypes.BOOL
    user32.SetTimer.argtypes = [
        wintypes.HWND, ctypes.c_size_t, wintypes.UINT, wintypes.LPVOID]
    user32.SetTimer.restype = ctypes.c_size_t
    user32.KillTimer.argtypes = [wintypes.HWND, ctypes.c_size_t]
    user32.KillTimer.restype = wintypes.BOOL
    user32.InvalidateRect.argtypes = [
        wintypes.HWND, ctypes.POINTER(wintypes.RECT), wintypes.BOOL]
    user32.InvalidateRect.restype = wintypes.BOOL
    user32.DestroyWindow.argtypes = [wintypes.HWND]
    user32.DestroyWindow.restype = wintypes.BOOL
    user32.PostQuitMessage.argtypes = [ctypes.c_int]
    user32.PostQuitMessage.restype = None
    user32.BeginPaint.argtypes = [wintypes.HWND, ctypes.POINTER(PAINTSTRUCT)]
    user32.BeginPaint.restype = wintypes.HDC
    user32.EndPaint.argtypes = [wintypes.HWND, ctypes.POINTER(PAINTSTRUCT)]
    user32.EndPaint.restype = wintypes.BOOL
    user32.GetClientRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
    user32.GetClientRect.restype = wintypes.BOOL
    user32.FillRect.argtypes = [
        wintypes.HDC, ctypes.POINTER(wintypes.RECT), wintypes.HBRUSH]
    user32.FillRect.restype = ctypes.c_int
    user32.DrawTextW.argtypes = [
        wintypes.HDC, wintypes.LPCWSTR, ctypes.c_int,
        ctypes.POINTER(wintypes.RECT), wintypes.UINT]
    user32.DrawTextW.restype = ctypes.c_int
    user32.GetMessageW.argtypes = [
        ctypes.POINTER(wintypes.MSG), wintypes.HWND, wintypes.UINT, wintypes.UINT]
    user32.GetMessageW.restype = wintypes.BOOL
    user32.TranslateMessage.argtypes = [ctypes.POINTER(wintypes.MSG)]
    user32.TranslateMessage.restype = wintypes.BOOL
    user32.DispatchMessageW.argtypes = [ctypes.POINTER(wintypes.MSG)]
    user32.DispatchMessageW.restype = LRESULT
    user32.GetWindowLongPtrW.argtypes = [wintypes.HWND, ctypes.c_int]
    user32.GetWindowLongPtrW.restype = ctypes.c_ssize_t
    gdi32.GetStockObject.argtypes = [ctypes.c_int]
    gdi32.GetStockObject.restype = wintypes.HGDIOBJ
    gdi32.SetTextColor.argtypes = [wintypes.HDC, wintypes.COLORREF]
    gdi32.SetTextColor.restype = wintypes.COLORREF
    gdi32.SetBkMode.argtypes = [wintypes.HDC, ctypes.c_int]
    gdi32.SetBkMode.restype = ctypes.c_int
    gdi32.CreateFontW.argtypes = [ctypes.c_int] * 5 + [wintypes.DWORD] * 8 + [
        wintypes.LPCWSTR]
    gdi32.CreateFontW.restype = wintypes.HFONT
    gdi32.SelectObject.argtypes = [wintypes.HDC, wintypes.HGDIOBJ]
    gdi32.SelectObject.restype = wintypes.HGDIOBJ
    gdi32.DeleteObject.argtypes = [wintypes.HGDIOBJ]
    gdi32.DeleteObject.restype = wintypes.BOOL

    font = gdi32.CreateFontW(
        -FONT_PX, 0, 0, 0, _FW_BOLD, 0, 0, 0, _DEFAULT_CHARSET,
        0, 0, 0, 0, "Segoe UI")
    started = time.monotonic()

    def wndproc(hwnd, msg, wparam, lparam):
        if msg == _WM_PAINT:
            ps = PAINTSTRUCT()
            hdc = user32.BeginPaint(hwnd, ctypes.byref(ps))
            rect = wintypes.RECT()
            user32.GetClientRect(hwnd, ctypes.byref(rect))
            user32.FillRect(hdc, ctypes.byref(rect),
                            gdi32.GetStockObject(_BLACK_BRUSH))
            gdi32.SetBkMode(hdc, _TRANSPARENT_BK)
            gdi32.SetTextColor(hdc, _WHITE)
            old_font = gdi32.SelectObject(hdc, font) if font else None
            user32.DrawTextW(hdc, label_text(), -1, ctypes.byref(rect),
                             _DT_CENTER | _DT_VCENTER | _DT_SINGLELINE)
            if old_font:
                gdi32.SelectObject(hdc, old_font)
            user32.EndPaint(hwnd, ctypes.byref(ps))
            return 0
        if msg == _WM_TIMER:
            if time.monotonic() - started >= LIFETIME_S:
                user32.DestroyWindow(hwnd)
            else:
                user32.InvalidateRect(hwnd, None, True)
            return 0
        if msg == _WM_DESTROY:
            user32.KillTimer(hwnd, _TIMER_ID)
            user32.PostQuitMessage(0)
            return 0
        return user32.DefWindowProcW(hwnd, msg, wparam, lparam)

    proc_ref = WNDPROC(wndproc)   # kept alive for as long as the window lives
    instance = kernel32.GetModuleHandleW(None)
    wc = WNDCLASSW()
    wc.lpfnWndProc = proc_ref
    wc.hInstance = instance
    wc.lpszClassName = _CLASS_NAME
    if not user32.RegisterClassW(ctypes.byref(wc)):
        out.write(f"RegisterClassW failed: error {ctypes.get_last_error()}\n")
        return 1
    try:
        x = (user32.GetSystemMetrics(_SM_CXSCREEN) - WINDOW_W) // 2
        hwnd = user32.CreateWindowExW(
            EX_STYLE, _CLASS_NAME, "Hydra overlay probe", WS_POPUP,
            x, 0, WINDOW_W, WINDOW_H, None, None, instance, None)
        if not hwnd:
            out.write(f"CreateWindowExW failed: error {ctypes.get_last_error()}\n")
            return 1
        user32.SetLayeredWindowAttributes(hwnd, 0, ALPHA, _LWA_ALPHA)
        if not user32.SetTimer(hwnd, _TIMER_ID, REPAINT_MS, None):
            out.write(f"SetTimer failed: error {ctypes.get_last_error()}; "
                      "closing the window now\n")
            user32.DestroyWindow(hwnd)
            return 1
        user32.ShowWindow(hwnd, _SW_SHOWNOACTIVATE)
        user32.UpdateWindow(hwnd)
        ex_style = user32.GetWindowLongPtrW(hwnd, _GWL_EXSTYLE) & 0xFFFFFFFF
        out.write(f"Window {hwnd:#x}, extended style {ex_style:#010x} "
                  f"(asked for {EX_STYLE:#010x}); topmost "
                  f"{'on' if ex_style & WS_EX_TOPMOST else 'OFF'}. "
                  f"Closes itself in {LIFETIME_S} s.\n")
        out.flush()

        msg = wintypes.MSG()
        while user32.GetMessageW(ctypes.byref(msg), None, 0, 0) > 0:
            user32.TranslateMessage(ctypes.byref(msg))
            user32.DispatchMessageW(ctypes.byref(msg))
        out.write(f"Window closed after {time.monotonic() - started:.1f} s.\n")
        return 0
    finally:
        user32.UnregisterClassW(_CLASS_NAME, instance)
        if font:
            gdi32.DeleteObject(font)


def main() -> int:
    return run_probe()


if __name__ == "__main__":
    sys.exit(main())
