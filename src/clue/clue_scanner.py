#!/usr/bin/env python3
"""
clue_scanner.py — Recursive file scanner for CTF-relevant clues.
Finds flags, keys, encoded strings, suspicious patterns.

Usage:
    clue_scanner.py <path> [--flags-only] [--entropy] [--all] [--depth N]
"""
import argparse
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from tokenizer import tokenize, T_FLAG, T_HEX_STRING, T_BASE64, T_URL, T_IP, T_EMAIL
from ngram_index import byte_entropy, classify_entropy

SKIP_EXTS = {".pyc", ".so", ".o", ".a", ".class", ".jar", ".exe",
             ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".mp3", ".mp4",
             ".avi", ".mkv", ".zip", ".gz", ".tar", ".7z"}

G  = "\033[32m"
Y  = "\033[33m"
C  = "\033[36m"
R  = "\033[31m"
X  = "\033[0m"
B  = "\033[1m"


def scan_file(path, flags_only, show_entropy, show_all):
    try:
        data = open(path, "rb").read()
    except (IOError, OSError):
        return

    tokens = list(tokenize(data))
    flags  = [t for t in tokens if t[0] == T_FLAG]
    others = [t for t in tokens if t[0] != T_FLAG]

    if not tokens and not show_entropy:
        return

    printed_header = [False]

    def header():
        if not printed_header[0]:
            print("")
            print(B + C + "[FILE] " + path + X)
            printed_header[0] = True

    for ttype, val, offset in flags:
        header()
        print("  " + G + B + "[FLAG @" + format(offset, "#010x") + "]" + X + "  " +
              val.decode("utf-8", errors="replace"))

    if show_entropy:
        header()
        e = byte_entropy(data)
        cls = classify_entropy(data)
        colour = R if e > 7.5 else Y if e > 6 else C
        print("  " + colour + "[ENTROPY] " + format(e, ".4f") + " bpb -- " + cls + X)

    if show_all or not flags_only:
        interesting = [t for t in others if t[0] in (T_URL, T_IP, T_EMAIL)]
        for ttype, val, offset in interesting:
            header()
            print("  " + Y + "[" + ttype.upper() + " @" + format(offset, "#010x") + "]" + X +
                  "  " + val.decode("utf-8", errors="replace")[:120])

    if show_all:
        for ttype, val, offset in others:
            if ttype in (T_HEX_STRING, T_BASE64):
                v = val.decode("utf-8", errors="replace")[:80]
                header()
                print("  [" + ttype.upper() + " @" + format(offset, "#010x") + "]  " + v)


def scan_path(root, flags_only, show_entropy, show_all, max_depth):
    root_path = Path(root)
    if root_path.is_file():
        scan_file(str(root_path), flags_only, show_entropy, show_all)
        return

    for path in sorted(root_path.rglob("*")):
        if path.is_dir():
            depth = len(path.relative_to(root_path).parts)
            if max_depth > 0 and depth > max_depth:
                continue
            continue
        if path.suffix.lower() in SKIP_EXTS:
            continue
        scan_file(str(path), flags_only, show_entropy, show_all)


def main():
    ap = argparse.ArgumentParser(description="CTF Clue Scanner")
    ap.add_argument("path", help="File or directory to scan")
    ap.add_argument("--flags-only", action="store_true")
    ap.add_argument("--entropy",    action="store_true")
    ap.add_argument("--all",        action="store_true")
    ap.add_argument("--depth",      type=int, default=0)
    args = ap.parse_args()

    print(B + "CTF Arsenal -- Clue Scanner v1.0" + X)
    print("Scanning: " + args.path)
    scan_path(args.path, args.flags_only, args.entropy, args.all, args.depth)
    print("")
    print(C + "Scan complete." + X)


if __name__ == "__main__":
    main()
