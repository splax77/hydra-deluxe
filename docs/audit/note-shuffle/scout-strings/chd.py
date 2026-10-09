"""Wrapper: py -E dis.py <il2cpp dir> <rva hex> [max_bytes hex] -> utf-8 stdout"""
import sys
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, sys.argv[1])
import chdis
chdis.dis(int(sys.argv[2], 16), int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x1200)
