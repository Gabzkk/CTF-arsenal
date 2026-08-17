#!/usr/bin/env bash
# test_rainbow.sh — Rainbow table module tests
set -uo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$REPO/bin"
PASS=0
FAIL=0

ok()  { echo -e "\033[32m[PASS]\033[0m $1"; PASS=$((PASS+1)); }
fail(){ echo -e "\033[31m[FAIL]\033[0m $1"; FAIL=$((FAIL+1)); }

echo "=== Rainbow Table Tests ==="

# Test 1: table_generator binary
if [[ -x "$BIN/table_generator" ]]; then
    ok "table_generator binary"
else
    fail "table_generator missing"
fi

# Test 2: table_query binary
if [[ -x "$BIN/table_query" ]]; then
    ok "table_query binary"
else
    fail "table_query missing"
fi

# Test 3: Generate a small table
TMP=$(mktemp)
if [[ -x "$BIN/table_generator" ]]; then
    "$BIN/table_generator" -a md5 -c 50 -n 2000 -l 4 -o "$TMP" 2>&1 | tail -3
    if [[ -s "$TMP" ]]; then
        ok "table_generator: created table ($(wc -c < "$TMP") bytes)"
    else
        fail "table_generator: empty output"
    fi
fi

# Test 4: Query for "test" MD5
if [[ -x "$BIN/table_query" && -s "$TMP" ]]; then
    result=$("$BIN/table_query" -t "$TMP" -H 098f6bcd4621d373cade4e832627b4f6 2>&1 || true)
    if echo "$result" | grep -qiE "(Cracked|Not found)"; then
        ok "table_query: ran successfully"
    else
        fail "table_query: unexpected output: $result"
    fi
fi

rm -f "$TMP"
echo "=== Results: $PASS passed, $FAIL failed ==="
[[ $FAIL -eq 0 ]] && exit 0 || exit 1
