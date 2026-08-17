#!/usr/bin/env python3
"""test_clue.py — Clue scanner module tests"""
import sys
import os
import tempfile

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "src", "clue"))

PASS = 0
FAIL = 0

def ok(msg):
    global PASS
    print(f"\033[32m[PASS]\033[0m {msg}")
    PASS += 1

def fail(msg):
    global FAIL
    print(f"\033[31m[FAIL]\033[0m {msg}")
    FAIL += 1

print("=== Clue Scanner Tests ===")

# ── Test 1: tokenizer import ───────────────────────────────────────────────────
try:
    from tokenizer import tokenize, T_FLAG
    ok("tokenizer import")
except ImportError as e:
    fail(f"tokenizer import: {e}")

# ── Test 2: flag detection ─────────────────────────────────────────────────────
try:
    from tokenizer import tokenize, T_FLAG
    data = b"Some data here flag{h3ll0_w0rld} more data"
    tokens = list(tokenize(data))
    flags = [t for t in tokens if t[0] == T_FLAG]
    if flags and b"flag{h3ll0_w0rld}" in flags[0][1]:
        ok("flag detection: found flag{h3ll0_w0rld}")
    else:
        fail(f"flag detection: expected flag, got {flags}")
except Exception as e:
    fail(f"flag detection: {e}")

# ── Test 3: entropy calculation ────────────────────────────────────────────────
try:
    from ngram_index import byte_entropy, classify_entropy
    uniform_data = bytes(range(256))
    e = byte_entropy(uniform_data)
    if 7.9 < e <= 8.0:
        ok(f"entropy: uniform data = {e:.4f} (expected ~8.0)")
    else:
        fail(f"entropy: uniform = {e:.4f}, expected ~8.0")
except Exception as e:
    fail(f"entropy: {e}")

# ── Test 4: low-entropy detection ─────────────────────────────────────────────
try:
    from ngram_index import byte_entropy, classify_entropy
    zeroes = bytes(1024)
    e = byte_entropy(zeroes)
    c = classify_entropy(zeroes)
    if e == 0.0 and "low_entropy" in c:
        ok(f"entropy: zero data = {e:.4f} ({c})")
    else:
        fail(f"entropy: zero data = {e:.4f}, classify = {c}")
except Exception as e:
    fail(f"entropy low: {e}")

# ── Test 5: clue_scanner on a temp file ────────────────────────────────────────
try:
    from clue_scanner import scan_path
    with tempfile.NamedTemporaryFile(suffix=".txt", delete=False, mode="wb") as f:
        f.write(b"Nothing here but HTB{s3cr3t_fl4g_f0r_t3st} and http://example.com")
        fname = f.name
    import io
    from contextlib import redirect_stdout
    buf = io.StringIO()
    with redirect_stdout(buf):
        scan_path(fname, flags_only=False, show_entropy=False,
                  show_all=True, max_depth=0)
    output = buf.getvalue()
    os.unlink(fname)
    if "HTB{s3cr3t_fl4g_f0r_t3st}" in output:
        ok("clue_scanner: detected HTB flag in file")
    else:
        fail(f"clue_scanner: flag not detected\nOutput: {output[:200]}")
except Exception as e:
    fail(f"clue_scanner: {e}")

print(f"=== Results: {PASS} passed, {FAIL} failed ===")
sys.exit(0 if FAIL == 0 else 1)
