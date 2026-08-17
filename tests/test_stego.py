#!/usr/bin/env python3
"""test_stego.py — Steganography module round-trip tests"""
import sys
import os
import tempfile
import struct
import wave

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "src", "stego"))

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

print("=== Steganography Tests ===")

# ── Test 1: PNG LSB encode/decode round-trip ───────────────────────────────────
try:
    from PIL import Image
    import png_lsb

    with tempfile.TemporaryDirectory() as tmp:
        # Create minimal test PNG
        img = Image.new("RGB", (100, 100), color=(128, 200, 64))
        src_path = os.path.join(tmp, "test.png")
        dst_path = os.path.join(tmp, "test_stego.png")
        img.save(src_path)

        secret = "CTF{png_lsb_t3st}"
        png_lsb.encode(src_path, secret, dst_path)
        recovered = png_lsb.decode(dst_path)
        if recovered == secret:
            ok(f"PNG LSB 1-bit round-trip: '{secret}'")
        else:
            fail(f"PNG LSB 1-bit: expected '{secret}', got '{recovered}'")

        # Test with 2-bit LSB
        dst2 = os.path.join(tmp, "test_stego2.png")
        png_lsb.encode(src_path, secret, dst2, bits=2)
        r2 = png_lsb.decode(dst2, bits=2)
        if r2 == secret:
            ok(f"PNG LSB 2-bit round-trip: '{secret}'")
        else:
            fail(f"PNG LSB 2-bit: expected '{secret}', got '{r2}'")

except Exception as e:
    fail(f"PNG LSB: {e}")

# ── Test 2: WAV LSB encode/decode round-trip ──────────────────────────────────
try:
    import wav_lsb

    with tempfile.TemporaryDirectory() as tmp:
        src_path = os.path.join(tmp, "test.wav")
        dst_path = os.path.join(tmp, "test_stego.wav")

        # Create minimal 16-bit mono WAV
        with wave.open(src_path, "wb") as wf:
            wf.setnchannels(1); wf.setsampwidth(2); wf.setframerate(44100)
            samples = struct.pack("<8192h", *([16383] * 8192))
            wf.writeframes(samples)

        secret = "CTF{wav_lsb_t3st}"
        wav_lsb.encode(src_path, secret, dst_path)
        recovered = wav_lsb.decode(dst_path)
        if recovered == secret:
            ok(f"WAV LSB round-trip: '{secret}'")
        else:
            fail(f"WAV LSB: expected '{secret}', got '{recovered}'")

except Exception as e:
    fail(f"WAV LSB: {e}")

# ── Test 3: JPEG EXIF dump (no crash) ─────────────────────────────────────────
try:
    from PIL import Image
    import jpg_exif
    import io
    from contextlib import redirect_stdout

    with tempfile.TemporaryDirectory() as tmp:
        img = Image.new("RGB", (50, 50), color=(255, 0, 0))
        src_path = os.path.join(tmp, "test.jpg")
        img.save(src_path, "JPEG")
        buf = io.StringIO()
        with redirect_stdout(buf):
            jpg_exif.dump(src_path)
        ok("JPEG EXIF dump: no crash")

except Exception as e:
    fail(f"JPEG EXIF: {e}")

print(f"=== Results: {PASS} passed, {FAIL} failed ===")
sys.exit(0 if FAIL == 0 else 1)
