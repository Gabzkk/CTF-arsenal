#!/usr/bin/env python3
"""
png_lsb.py — PNG LSB steganography (encode/decode)
Uses Pillow. Supports 1-4 LSB bits per channel.
"""
import struct
import sys
from PIL import Image


DELIMITER = b"\x00\x00\x00"  # 3 null bytes mark end of message


def _bits_to_bytes(bits):
    """Convert list of bits to bytes."""
    out = bytearray()
    for i in range(0, len(bits), 8):
        chunk = bits[i:i+8]
        if len(chunk) < 8:
            break
        out.append(int("".join(str(b) for b in chunk), 2))
    return bytes(out)


def encode(src: str, message: str, dst: str, bits: int = 1):
    """Encode message string into PNG using LSB steganography."""
    img = Image.open(src).convert("RGB")
    pixels = list(img.getdata())

    msg_bytes = message.encode("utf-8") + DELIMITER
    msg_bits = []
    for byte in msg_bytes:
        for bit in range(7, -1, -1):
            msg_bits.append((byte >> bit) & 1)

    mask = (1 << bits) - 1
    capacity = len(pixels) * 3 * bits
    if len(msg_bits) > capacity:
        print(f"[!] Message too large ({len(msg_bits)} bits > {capacity} capacity)", file=sys.stderr)
        sys.exit(1)

    bit_idx = 0
    new_pixels = []
    for pixel in pixels:
        new_ch = []
        for channel in pixel:
            if bit_idx < len(msg_bits):
                chunk = 0
                for _ in range(bits):
                    chunk = (chunk << 1) | (msg_bits[bit_idx] if bit_idx < len(msg_bits) else 0)
                    bit_idx += 1
                channel = (channel & ~mask) | chunk
            new_ch.append(channel)
        new_pixels.append(tuple(new_ch))

    out_img = Image.new("RGB", img.size)
    out_img.putdata(new_pixels)
    out_img.save(dst)
    print(f"[+] Encoded {len(message)} chars into {dst} using {bits}-bit LSB")


def decode(src: str, bits: int = 1) -> str:
    """Decode LSB-hidden message from PNG."""
    img = Image.open(src).convert("RGB")
    pixels = list(img.getdata())
    mask = (1 << bits) - 1

    bits_list = []
    for pixel in pixels:
        for channel in pixel:
            for b in range(bits - 1, -1, -1):
                bits_list.append((channel >> b) & 1)

    raw = _bits_to_bytes(bits_list)
    # Find delimiter
    idx = raw.find(DELIMITER)
    if idx == -1:
        return raw.decode("utf-8", errors="replace")
    return raw[:idx].decode("utf-8", errors="replace")


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="PNG LSB Steganography")
    ap.add_argument("action", choices=["encode", "decode"])
    ap.add_argument("src"); ap.add_argument("--msg"); ap.add_argument("--out")
    ap.add_argument("--bits", type=int, default=1)
    a = ap.parse_args()
    if a.action == "encode":
        encode(a.src, a.msg, a.out or "out.png", a.bits)
    else:
        print(decode(a.src, a.bits))
