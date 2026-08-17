#!/usr/bin/env bash
# test_brute.sh — Brute-force module & SecLists integration tests
set -uo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$REPO/bin"
WL="$REPO/wordlists/rockyou_sample.txt"
SEC_WL="$REPO/wordlists/seclists/passwords/top10k.txt"
PASS=0
FAIL=0

ok()  { echo -e "\033[32m[PASS]\033[0m $1"; PASS=$((PASS+1)); }
fail(){ echo -e "\033[31m[FAIL]\033[0m $1"; FAIL=$((FAIL+1)); }

echo "=== Brute-Force Tests ==="

# Test 1: zip_cracker binary exists
if [[ -x "$BIN/zip_cracker" ]]; then
    ok "zip_cracker binary exists"
else
    fail "zip_cracker binary missing"
fi

# Test 2: Create a test ZIP and crack it with built-in sample
TMPDIR_T=$(mktemp -d)
TEST_ZIP="$TMPDIR_T/test.zip"
TEST_PASS="password"
echo "CTF_TEST_CONTENT" > "$TMPDIR_T/secret.txt"
if zip -j -P "$TEST_PASS" "$TEST_ZIP" "$TMPDIR_T/secret.txt" > /dev/null 2>&1; then
    if [[ -x "$BIN/zip_cracker" ]]; then
        if "$BIN/zip_cracker" -z "$TEST_ZIP" -w "$WL" 2>&1 | grep -q "password"; then
            ok "zip_cracker: cracked test ZIP with password '$TEST_PASS'"
        else
            fail "zip_cracker: failed to crack test ZIP"
        fi
    fi
else
    fail "zip not available to create test archive"
fi

# Test 3: SecLists integration crack with alias
if [[ -f "$SEC_WL" && -x "$BIN/zip_cracker" ]]; then
    TEST_ZIP2="$TMPDIR_T/test2.zip"
    echo "SECRET2" > "$TMPDIR_T/sec2.txt"
    zip -j -P "dragon" "$TEST_ZIP2" "$TMPDIR_T/sec2.txt" > /dev/null 2>&1
    if "$REPO/scripts/ctf_arsenal.sh" brute zip -z "$TEST_ZIP2" -w top10k 2>&1 | grep -q "dragon"; then
        ok "SecLists integration: cracked with alias 'top10k'"
    else
        fail "SecLists integration: failed with alias 'top10k'"
    fi
fi

# Test 4: multi_brute binary exists and cracks with SecLists
if [[ -x "$BIN/multi_brute" ]]; then
    ok "multi_brute binary exists"
    if [[ -f "$SEC_WL" ]]; then
        if "$REPO/scripts/ctf_arsenal.sh" brute multi -t "$TEST_ZIP2" -w best1050 -j 4 2>&1 | grep -q "dragon"; then
            ok "multi_brute: multi-threaded crack with SecLists alias 'best1050'"
        else
            fail "multi_brute: failed to crack with 'best1050'"
        fi
    fi
else
    fail "multi_brute binary missing"
fi

# Test 5: pdf_cracker binary exists
if [[ -x "$BIN/pdf_cracker" ]]; then
    ok "pdf_cracker binary exists"
else
    fail "pdf_cracker binary missing"
fi

rm -rf "$TMPDIR_T"

echo "=== Results: $PASS passed, $FAIL failed ==="
[[ $FAIL -eq 0 ]] && exit 0 || exit 1
