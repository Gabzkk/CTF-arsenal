#!/usr/bin/env python3
"""
tokenizer.py — Byte-stream and text tokenizer for clue scanning.
Produces a stream of (type, value, offset) tokens.
"""
import re
from typing import Iterator, Tuple

T_PRINTABLE  = "printable"
T_HEX_STRING = "hex_str"
T_BASE64     = "base64"
T_FLAG       = "flag"
T_URL        = "url"
T_IP         = "ip"
T_EMAIL      = "email"
T_BINARY_SEQ = "bin_seq"

RE_FLAG    = re.compile(rb'(?:flag|ctf|HTB|picoCTF|DUCTF|FLAG)\{[^}]{1,128}\}', re.IGNORECASE)
RE_HEX     = re.compile(rb'(?:[0-9a-fA-F]{2}){4,}')
RE_BASE64  = re.compile(rb'[A-Za-z0-9+/]{20,}={0,2}')
RE_URL     = re.compile(rb'https?://[^\s<>"\']{4,128}')
RE_IP      = re.compile(rb'\b(?:\d{1,3}\.){3}\d{1,3}\b')
RE_EMAIL   = re.compile(rb'[\w.+-]+@[\w-]+\.[a-z]{2,6}', re.IGNORECASE)
RE_PRINT   = re.compile(rb'[\x20-\x7e]{6,}')


def tokenize(data: bytes) -> Iterator[Tuple[str, bytes, int]]:
    """Yield (token_type, value, byte_offset) from raw bytes."""
    seen_spans = []

    def emit_non_overlapping(pattern, ttype):
        for m in pattern.finditer(data):
            span = (m.start(), m.end())
            if not any(s[0] <= span[0] < s[1] for s in seen_spans):
                seen_spans.append(span)
                yield (ttype, m.group(), m.start())

    yield from emit_non_overlapping(RE_FLAG, T_FLAG)
    yield from emit_non_overlapping(RE_URL, T_URL)
    yield from emit_non_overlapping(RE_IP, T_IP)
    yield from emit_non_overlapping(RE_EMAIL, T_EMAIL)
    yield from emit_non_overlapping(RE_HEX, T_HEX_STRING)
    yield from emit_non_overlapping(RE_BASE64, T_BASE64)
    yield from emit_non_overlapping(RE_PRINT, T_PRINTABLE)
