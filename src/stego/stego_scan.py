#!/usr/bin/env python3
"""
stego_scan.py — CTF Steganography dispatcher
Detects file type and routes to the appropriate stego sub-module.

Usage:
    stego_scan.py <file> [--decode] [--encode <message>] [--lsb-bits N]
    stego_scan.py <file> --exif-dump
    stego_scan.py <file> --entropy
"""
import argparse
import sys
import os
import math
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import png_lsb
import jpg_exif
import wav_lsb


def file_entropy(path: str) -> float:
    """Shannon entropy of the file bytes (bits per byte)."""
    data = Path(path).read_bytes()
    if not data:
        return 0.0
    freq = [0] * 256
    for b in data:
        freq[b] += 1
    n = len(data)
    return -sum((c/n) * math.log2(c/n) for c in freq if c > 0)


def detect_type(path: str) -> str:
    header = open(path, "rb").read(8)
    if header[:4] == b"\x89PNG":
        return "png"
    if header[:2] == b"\xff\xd8":
        return "jpg"
    if header[:4] == b"RIFF":
        return "wav"
    return "unknown"


def main():
    ap = argparse.ArgumentParser(description="CTF Steganography Scanner")
    ap.add_argument("file", help="Target file")
    ap.add_argument("--decode",       action="store_true", help="Decode hidden message")
    ap.add_argument("--encode",       metavar="MSG",       help="Encode message into file")
    ap.add_argument("--output",       metavar="OUT",       help="Output file for encoding")
    ap.add_argument("--lsb-bits",     type=int, default=1, help="LSB bits to use (1-4)")
    ap.add_argument("--exif-dump",    action="store_true", help="Dump all EXIF metadata")
    ap.add_argument("--exif-inject",  metavar="DATA",      help="Inject data into EXIF UserComment")
    ap.add_argument("--entropy",      action="store_true", help="Print Shannon entropy")
    args = ap.parse_args()

    if not os.path.exists(args.file):
        print(f"[!] File not found: {args.file}", file=sys.stderr)
        sys.exit(1)

    ftype = detect_type(args.file)
    print(f"[-] File type detected: {ftype.upper()}")

    if args.entropy:
        e = file_entropy(args.file)
        flag = "  *** HIGH ENTROPY — likely encrypted/compressed ***" if e > 7.5 else ""
        print(f"[-] Shannon entropy: {e:.4f} bits/byte{flag}")
        return

    if ftype == "png":
        if args.encode:
            out = args.output or args.file.replace(".png", "_stego.png")
            png_lsb.encode(args.file, args.encode, out, bits=args.lsb_bits)
        elif args.decode:
            msg = png_lsb.decode(args.file, bits=args.lsb_bits)
            print(f"[+] Decoded message: {msg}")
        else:
            print("[-] No action specified. Use --encode or --decode.")

    elif ftype == "jpg":
        if args.exif_dump:
            jpg_exif.dump(args.file)
        elif args.exif_inject:
            out = args.output or args.file.replace(".jpg", "_stego.jpg")
            jpg_exif.inject(args.file, args.exif_inject, out)
        elif args.decode:
            jpg_exif.dump(args.file)
        else:
            print("[-] JPEG: use --exif-dump or --exif-inject.")

    elif ftype == "wav":
        if args.encode:
            out = args.output or args.file.replace(".wav", "_stego.wav")
            wav_lsb.encode(args.file, args.encode, out)
        elif args.decode:
            msg = wav_lsb.decode(args.file)
            print(f"[+] Decoded message: {msg}")
        else:
            print("[-] WAV: use --encode or --decode.")

    else:
        print(f"[!] Unsupported file type: {ftype}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
