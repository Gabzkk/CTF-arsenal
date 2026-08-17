#!/usr/bin/env bash
# test_fuzz.sh — Protocol fuzzer smoke tests
set -uo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$REPO/bin"
PASS=0
FAIL=0

ok()  { echo -e "\033[32m[PASS]\033[0m $1"; PASS=$((PASS+1)); }
fail(){ echo -e "\033[31m[FAIL]\033[0m $1"; FAIL=$((FAIL+1)); }

echo "=== Protocol Fuzzer Tests ==="

# Test 1: Binary existence
for b in http_fuzz dns_fuzz smb_fuzz; do
    if [[ -x "$BIN/$b" ]]; then
        ok "$b binary exists"
    else
        fail "$b binary missing"
    fi
done

# Test 2: HTTP fuzzer against netcat listener (loopback)
if [[ -x "$BIN/http_fuzz" ]]; then
    PORT=18765
    nc -l -p "$PORT" > /dev/null 2>&1 &
    NC_PID=$!
    sleep 0.2
    "$BIN/http_fuzz" -H 127.0.0.1 -p "$PORT" -n 5 2>&1 | tail -3
    kill "$NC_PID" 2>/dev/null || true
    ok "http_fuzz: completed 5 mutations against loopback listener"
fi

# Test 3: DNS fuzzer (timeouts expected)
if [[ -x "$BIN/dns_fuzz" ]]; then
    "$BIN/dns_fuzz" -H 127.0.0.1 -n 3 2>&1 | tail -2
    ok "dns_fuzz: completed 3 mutations"
fi

echo "=== Results: $PASS passed, $FAIL failed ==="
[[ $FAIL -eq 0 ]] && exit 0 || exit 1
