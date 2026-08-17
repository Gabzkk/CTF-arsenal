# CTF Arsenal 🛡️

**A POSIX-compliant, modular CTF toolkit in C99 + Python 3**  
Integrated with **SecLists** 
Covers: Brute-force · Rainbow Tables · Steganography · Clue Scanning · Protocol Fuzzing · Wordlists

> Public Domain — Unlicense | Built for Manjaro/Arch Linux (GCC 16, Python 3.14)

---

## Quick Start

```bash
# 1. Install dependencies
bash scripts/install_deps.sh

# 2. Build all C binaries
make all

# 3. View available modules & integrated wordlists
./scripts/ctf_arsenal.sh help
./scripts/ctf_arsenal.sh wordlists
```

---

## 📚 Integrated SecLists Dictionaries

CTF-Arsenal natively integrates with `/home/sunburnz/Desktop/HAckTools/SecLists` through structured symlinks and automatic CLI shorthand resolution.

| Alias | Target Wordlist | Purpose |
|---|---|---|
| `top1k` | `Passwords/Common-Credentials/Pwdb_top-1000.txt` | Fast triage password list (1k words) |
| `top10k` | `Passwords/Common-Credentials/10k-most-common.txt` | High-probability passwords (10k words) |
| `top100k` | `Passwords/Common-Credentials/100k-most-used-passwords-NCSC.txt` | Deep dictionary coverage (100k words) |
| `top1m` | `Passwords/Common-Credentials/xato-net-10-million-passwords-1000000.txt` | 1 Million passwords |
| `best1050` | `Passwords/Common-Credentials/best1050.txt` | High-hitrate CTF password list |
| `defaults` | `Passwords/Default-Credentials/default-passwords.txt` | Default appliance & service credentials |
| `darkweb` | `Passwords/Common-Credentials/darkweb2017_top-1000.txt` | Real-world breach passwords |
| `rockyou` | `~/Desktop/HAckTools/rockyou.txt` | Full 14M password list (139 MB) |
| `web` | `Discovery/Web-Content/common.txt` | Common directories & files |
| `raft_dirs` | `Discovery/Web-Content/raft-medium-directories.txt` | Raft directory fuzzing |
| `burp_params`| `Discovery/Web-Content/burp-parameter-names.txt` | Parameter name fuzzing |
| `subdomains` | `Discovery/DNS/subdomains-top1million-5000.txt` | Top 5,000 subdomains |
| `lfi` | `Fuzzing/LFI/LFI-Jhaddix.txt` | Path traversal & LFI probes |
| `xss` | `Fuzzing/XSS/human-friendly/XSS-Jhaddix.txt` | Contextual XSS payloads |
| `sqli` | `Fuzzing/Databases/SQLi/Generic-SQLi.txt` | SQL injection test strings |
| `ai_escape` | `Ai/LLM_Testing/Divergence_attack/...` | LLM alignment break prompts |

---

## Modules & Usage

### 🔐 Brute-Force (`brute`)

Multi-threaded wordlist + mutation cracker for ZIP, RAR, and PDF.

```bash
# Crack a ZIP using SecLists top 10k shorthand
./scripts/ctf_arsenal.sh brute zip -z challenge.zip -w top10k

# Crack with best1050 and mutations
./scripts/ctf_arsenal.sh brute zip -z challenge.zip -w best1050 -m wordlists/mutations.txt

# Multi-threaded brute with 8 threads using default credentials list
./scripts/ctf_arsenal.sh brute multi -t secret.pdf -w defaults -j 8

# Using full rockyou.txt
./scripts/ctf_arsenal.sh brute zip -z archive.zip -w rockyou
```

### 🌈 Rainbow Tables (`rainbow`)

Generate MD5/SHA1/SHA256 rainbow tables and query them.

```bash
# Generate table (10k chains, MD5, 6-char plaintexts)
./scripts/ctf_arsenal.sh rainbow gen -a md5 -n 10000 -c 100 -l 6 -o my_table.rt

# Query: crack the MD5 of "password" 
./scripts/ctf_arsenal.sh rainbow query -t my_table.rt \
  -H 5f4dcc3b5aa765d61d8327deb882cf99
```

### 🖼️ Steganography (`stego`)

LSB encode/decode for PNG & WAV; EXIF inspection for JPEG.

```bash
# Encode a secret message into a PNG
./scripts/ctf_arsenal.sh stego flag.png --encode "CTF{hidden_message}" --output out.png

# Decode from PNG
./scripts/ctf_arsenal.sh stego out.png --decode

# Check file entropy (high = encrypted/compressed?)
./scripts/ctf_arsenal.sh stego suspicious.bin --entropy
```

### 🔍 Clue Scanner (`clue`)

Recursively scans files/directories for flags, URLs, IPs, base64, hex strings.

```bash
# Scan for flags only
./scripts/ctf_arsenal.sh clue /path/to/challenge --flags-only

# Full scan with entropy analysis
./scripts/ctf_arsenal.sh clue /path/to/challenge --entropy --all
```

### 🌐 Protocol Fuzzer (`fuzz`)

Sends mutated frames to HTTP, DNS, or SMB services.

```bash
# HTTP fuzzer (200 mutations)
./scripts/ctf_arsenal.sh fuzz http -H 127.0.0.1 -p 8080 -P /api/login -n 200

# DNS fuzzer (50 mutations against resolver)
./scripts/ctf_arsenal.sh fuzz dns -H 8.8.8.8 -n 50
```

---

## Directory Structure

```
ctf_arsenal/
├── Makefile                  # Top-level build
├── README.md
├── src/
│   ├── core/                 # base.h, utils.h, utils.c
│   ├── brute/                # multi_brute.c, zip_cracker.c, rar_cracker.c, pdf_cracker.c
│   ├── rainbow/              # hash_rainbow.c, table_generator.c, table_query.c
│   ├── stego/                # stego_scan.py, png_lsb.py, jpg_exif.py, wav_lsb.py
│   ├── clue/                 # clue_scanner.py, tokenizer.py, ngram_index.py
│   └── fuzz/                 # proto_fuzz.c, http_fuzz.c, dns_fuzz.c, smb_fuzz.c
├── bin/                      # Compiled ELF binaries (git-ignored)
├── scripts/
│   ├── ctf_arsenal.sh        # Master CLI with SecLists resolver
│   ├── install_deps.sh       # Dependency installer
│   └── docker_build.sh       # Docker containerisation
├── wordlists/
│   ├── rockyou_sample.txt    # Curated sample
│   ├── common_ctf.txt        # CTF-specific flags & keywords
│   ├── mutations.txt         # Suffix/prefix mutation rules
│   └── seclists/             # 30+ structured links to SecLists
│       ├── passwords/        # top1k, top10k, top100k, top1m, best1050, defaults, rockyou
│       ├── discovery/        # common_web, raft_dirs, raft_files, burp_params, subdomains
│       ├── fuzzing/          # lfi_jhaddix, xss_jhaddix, sqli_generic, sqli_auth_bypass
│       ├── usernames/        # top_shortlist, cirt_default, names
│       └── ai_llm/           # divergence_escape, data_leakage, session_recall
├── config/
│   ├── arsenal.conf          # Global settings & SecLists aliases
│   └── rainbow.conf          # Rainbow table parameters
└── tests/
    ├── test_brute.sh         # Brute-force & SecLists unit tests
    ├── test_clue.py          # Clue & entropy tests
    ├── test_rainbow.sh       # Rainbow table tests
    ├── test_stego.py         # Steganography round-trip tests
    └── test_fuzz.sh          # Protocol fuzzing smoke tests
```

---

## Building & Verification

```bash
make all          # Build all C binaries → bin/
make test         # Run all 5 test suites (23/23 tests pass)
make clean        # Remove bin/ and generated tables
```

---

## License

Public Domain — Unlicense.
