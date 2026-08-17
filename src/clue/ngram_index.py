#!/usr/bin/env python3
"""
ngram_index.py — Bigram/trigram frequency analysis for entropy and language detection.
"""
import math
import re
from collections import Counter
from typing import Dict, List


# Approximate English bigram log-probabilities (top 20 only, rest get default)
ENGLISH_BIGRAMS = {
    "th": -2.7, "he": -2.9, "in": -3.0, "er": -3.1, "an": -3.2,
    "re": -3.3, "on": -3.4, "at": -3.5, "en": -3.6, "nd": -3.7,
    "ti": -3.8, "es": -3.9, "or": -4.0, "te": -4.1, "of": -4.2,
    "ed": -4.3, "is": -4.4, "it": -4.5, "al": -4.6, "ar": -4.7,
}
BIGRAM_DEFAULT = -8.0


def byte_entropy(data: bytes) -> float:
    """Shannon entropy in bits per byte."""
    if not data:
        return 0.0
    freq = Counter(data)
    n = len(data)
    return -sum((c/n) * math.log2(c/n) for c in freq.values())


def char_bigram_score(text: str) -> float:
    """
    English language likelihood score using bigram log-probabilities.
    Higher (less negative) = more English-like.
    """
    text = text.lower()
    text = re.sub(r"[^a-z ]", "", text)
    if len(text) < 4:
        return BIGRAM_DEFAULT * 4
    score = 0.0
    for i in range(len(text) - 1):
        bg = text[i:i+2]
        score += ENGLISH_BIGRAMS.get(bg, BIGRAM_DEFAULT)
    return score / max(1, len(text) - 1)


def classify_entropy(data: bytes) -> str:
    e = byte_entropy(data)
    if e < 3.5:
        return "low_entropy (structured/repetitive)"
    elif e < 6.0:
        return "medium_entropy (text-like)"
    elif e < 7.5:
        return "high_entropy (compressed?)"
    else:
        return "very_high_entropy (encrypted?)"


def top_ngrams(data: bytes, n: int = 2, top_k: int = 20) -> List:
    grams = []
    for i in range(len(data) - n + 1):
        grams.append(data[i:i+n])
    counter = Counter(grams)
    return counter.most_common(top_k)
