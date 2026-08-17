#!/usr/bin/env python3
"""
wav_lsb.py — WAV LSB steganography (encode/decode)
Uses stdlib wave module. Modifies LSB of 16-bit samples.
"""
import wave
import struct
import sys


DELIMITER = b"\x00\x00\x00"


def encode(src: str, message: str, dst: str):
    """Encode message into WAV LSB."""
    msg_bytes = message.encode("utf-8") + DELIMITER
    msg_bits = []
    for byte in msg_bytes:
        for bit in range(7, -1, -1):
            msg_bits.append((byte >> bit) & 1)

    with wave.open(src, "rb") as wf:
        params = wf.getparams()
        frames = wf.readframes(wf.getnframes())

    sampwidth = params.sampwidth
    if sampwidth != 2:
        print(f"[!] Only 16-bit WAV supported (got {sampwidth*8}-bit)", file=sys.stderr)
        sys.exit(1)

    samples = list(struct.unpack(f"<{len(frames)//2}h", frames))
    if len(msg_bits) > len(samples):
        print("[!] Message too large for this WAV", file=sys.stderr)
        sys.exit(1)

    for i, bit in enumerate(msg_bits):
        samples[i] = (samples[i] & ~1) | bit

    new_frames = struct.pack(f"<{len(samples)}h", *samples)
    with wave.open(dst, "wb") as wf_out:
        wf_out.setparams(params)
        wf_out.writeframes(new_frames)
    print(f"[+] Encoded {len(message)} chars into {dst}")


def decode(src: str) -> str:
    """Decode LSB-hidden message from WAV."""
    with wave.open(src, "rb") as wf:
        frames = wf.readframes(wf.getnframes())

    samples = struct.unpack(f"<{len(frames)//2}h", frames)
    bits = [s & 1 for s in samples]

    raw = bytearray()
    for i in range(0, len(bits) - 7, 8):
        byte = int("".join(str(b) for b in bits[i:i+8]), 2)
        raw.append(byte)
        if raw[-3:] == bytearray(DELIMITER):
            return raw[:-3].decode("utf-8", errors="replace")
    return raw.decode("utf-8", errors="replace")


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="WAV LSB Steganography")
    ap.add_argument("action", choices=["encode", "decode"])
    ap.add_argument("src"); ap.add_argument("--msg"); ap.add_argument("--out")
    a = ap.parse_args()
    if a.action == "encode":
        encode(a.src, a.msg, a.out or "out.wav")
    else:
        print(decode(a.src))
