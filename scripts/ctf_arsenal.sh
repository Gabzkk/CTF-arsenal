#!/usr/bin/env bash
# ctf_arsenal.sh — Master CTF Arsenal CLI dispatcher
# Usage: ctf_arsenal.sh <module> [module-args...]
# Modules: brute, rainbow, stego, clue, fuzz, wordlists, help

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARSENAL_ROOT="$(dirname "$SCRIPT_DIR")"
BIN_DIR="$ARSENAL_ROOT/bin"
SRC_DIR="$ARSENAL_ROOT/src"
WL_DIR="$ARSENAL_ROOT/wordlists"
SEC_DIR="$WL_DIR/seclists"

# Colours
RED='\033[31m'; GREEN='\033[32m'; CYAN='\033[36m'; YELLOW='\033[33m'
BOLD='\033[1m'; RESET='\033[0m'

banner() {
    echo -e "${BOLD}${CYAN}"
    echo " ██████╗████████╗███████╗      █████╗ ██████╗ ███████╗███████╗███╗   ██╗ █████╗ ██╗     "
    echo "██╔════╝╚══██╔══╝██╔════╝     ██╔══██╗██╔══██╗██╔════╝██╔════╝████╗  ██║██╔══██╗██║     "
    echo "██║        ██║   █████╗       ███████║██████╔╝███████╗█████╗  ██╔██╗ ██║███████║██║     "
    echo "██║        ██║   ██╔══╝       ██╔══██║██╔══██╗╚════██║██╔══╝  ██║╚██╗██║██╔══██║██║     "
    echo "╚██████╗   ██║   ██║          ██║  ██║██║  ██║███████║███████╗██║ ╚████║██║  ██║███████╗"
    echo " ╚═════╝   ╚═╝   ╚═╝          ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝"
    echo -e "${RESET}"
    echo -e "${BOLD}CTF Arsenal v1.1.0 | SecLists Integrated | github.com/ctf-arsenal${RESET}"
    echo
}

# Resolve shorthand aliases to full SecLists paths
resolve_wordlist() {
    local target="$1"
    # Strip prefix if given
    target="${target#seclists:}"

    case "$target" in
        top10k|10k)       echo "$SEC_DIR/passwords/top10k.txt" ;;
        top1k|1k)         echo "$SEC_DIR/passwords/top1k.txt" ;;
        top100k|100k)     echo "$SEC_DIR/passwords/top100k.txt" ;;
        top1m|1m)         echo "$SEC_DIR/passwords/top1m.txt" ;;
        best1050)         echo "$SEC_DIR/passwords/best1050.txt" ;;
        darkweb)          echo "$SEC_DIR/passwords/darkweb_top1k.txt" ;;
        defaults)         echo "$SEC_DIR/passwords/default_passwords.txt" ;;
        cirt)             echo "$SEC_DIR/passwords/cirt_collection.txt" ;;
        rockyou)          echo "$SEC_DIR/passwords/rockyou.txt" ;;
        web|common_web)   echo "$SEC_DIR/discovery/common_web.txt" ;;
        raft_dirs)        echo "$SEC_DIR/discovery/raft_dirs.txt" ;;
        raft_files)       echo "$SEC_DIR/discovery/raft_files.txt" ;;
        burp_params)      echo "$SEC_DIR/discovery/burp_params.txt" ;;
        subdomains)       echo "$SEC_DIR/discovery/subdomains_5k.txt" ;;
        lfi)              echo "$SEC_DIR/fuzzing/lfi_jhaddix.txt" ;;
        xss)              echo "$SEC_DIR/fuzzing/xss_jhaddix.txt" ;;
        sqli)             echo "$SEC_DIR/fuzzing/sqli_generic.txt" ;;
        usernames)        echo "$SEC_DIR/usernames/top_shortlist.txt" ;;
        ai_escape)        echo "$SEC_DIR/ai_llm/divergence_escape.txt" ;;
        ai_leakage)       echo "$SEC_DIR/ai_llm/data_leakage.txt" ;;
        sample|rockyou_sample) echo "$WL_DIR/rockyou_sample.txt" ;;
        ctf|common_ctf)   echo "$WL_DIR/common_ctf.txt" ;;
        *)
            if [[ -f "$1" ]]; then
                echo "$1"
            elif [[ -f "$WL_DIR/$1" ]]; then
                echo "$WL_DIR/$1"
            elif [[ -f "$SEC_DIR/$1" ]]; then
                echo "$SEC_DIR/$1"
            else
                echo "$1"
            fi
            ;;
    esac
}

# Translate args to substitute wordlist aliases in -w / --wordlist
filter_args() {
    local out=()
    while [[ $# -gt 0 ]]; do
        if [[ "$1" == "-w" && $# -gt 1 ]]; then
            out+=("-w" "$(resolve_wordlist "$2")")
            shift 2
        elif [[ "$1" == --wordlist=* ]]; then
            local val="${1#*=}"
            out+=("--wordlist=$(resolve_wordlist "$val")")
            shift
        else
            out+=("$1")
            shift
        fi
    done
    printf '%s\n' "${out[@]}"
}

list_wordlists() {
    banner
    echo -e "${BOLD}${GREEN}=== Integrated SecLists & CTF Dictionaries ===${RESET}"
    echo
    echo -e "${BOLD}🔑 Passwords & Credentials:${RESET}"
    printf "  %-18s -> %s\n" "top1k" "Pwdb Top 1,000 Common Passwords"
    printf "  %-18s -> %s\n" "top10k" "10k Most Common Passwords"
    printf "  %-18s -> %s\n" "top100k" "NCSC 100k Most Used Passwords"
    printf "  %-18s -> %s\n" "top1m" "xato-net 1 Million Passwords"
    printf "  %-18s -> %s\n" "best1050" "Best 1,050 Passwords"
    printf "  %-18s -> %s\n" "defaults" "Comprehensive Default Passwords list"
    printf "  %-18s -> %s\n" "darkweb" "Darkweb 2017 Top 1,000 Passwords"
    printf "  %-18s -> %s\n" "rockyou" "Full rockyou.txt (139 MB / 14M words)"
    printf "  %-18s -> %s\n" "sample" "Curated rockyou sample"
    echo
    echo -e "${BOLD}🌐 Web Content & Subdomain Discovery:${RESET}"
    printf "  %-18s -> %s\n" "web" "SecLists common.txt"
    printf "  %-18s -> %s\n" "raft_dirs" "Raft Medium Directories"
    printf "  %-18s -> %s\n" "raft_files" "Raft Medium Files"
    printf "  %-18s -> %s\n" "burp_params" "Burp Suite Parameter Names"
    printf "  %-18s -> %s\n" "subdomains" "Top 5,000 Subdomains"
    echo
    echo -e "${BOLD}🎯 Vulnerability Fuzzing Payloads:${RESET}"
    printf "  %-18s -> %s\n" "lfi" "LFI Jhaddix Payloads"
    printf "  %-18s -> %s\n" "xss" "XSS Jhaddix Contextual Payloads"
    printf "  %-18s -> %s\n" "sqli" "Generic SQL Injection Payloads"
    echo
    echo -e "${BOLD}🤖 AI & LLM Adversarial Testing:${RESET}"
    printf "  %-18s -> %s\n" "ai_escape" "LLM Divergence & Alignment Escape Prompts"
    printf "  %-18s -> %s\n" "ai_leakage" "Personal Data & System Prompt Leakage Probes"
    echo
    echo -e "${CYAN}Usage with shorthand:${RESET} ctf_arsenal.sh brute zip -z flag.zip -w top10k"
}

usage() {
    banner
    echo -e "${BOLD}Usage:${RESET} ctf_arsenal.sh <module> [options]"
    echo
    echo -e "${BOLD}Modules:${RESET}"
    echo -e "  ${GREEN}brute${RESET}     zip|rar|pdf|multi — password brute-force (SecLists aliases supported)"
    echo -e "  ${GREEN}rainbow${RESET}   gen|query         — rainbow table operations"
    echo -e "  ${GREEN}stego${RESET}                       — steganography (PNG/JPEG/WAV)"
    echo -e "  ${GREEN}clue${RESET}                        — file clue & flag scanner"
    echo -e "  ${GREEN}fuzz${RESET}     http|dns|smb       — protocol fuzzer"
    echo -e "  ${GREEN}wordlists${RESET}                  — list all integrated SecLists & dictionaries"
    echo -e "  ${GREEN}help${RESET}                        — show this message"
    echo
    echo -e "${BOLD}Examples with SecLists aliases:${RESET}"
    echo "  ctf_arsenal.sh brute zip -z archive.zip -w top10k"
    echo "  ctf_arsenal.sh brute zip -z archive.zip -w best1050 -m wordlists/mutations.txt"
    echo "  ctf_arsenal.sh brute multi -t secret.pdf -w defaults -j 8"
    echo "  ctf_arsenal.sh rainbow gen -a md5 -n 10000 -o rainbow.rt"
    echo "  ctf_arsenal.sh clue /path/to/challenge --entropy --all"
    echo "  ctf_arsenal.sh wordlists"
}

build_if_needed() {
    local bin="$BIN_DIR/$1"
    if [[ ! -x "$bin" ]]; then
        echo -e "${YELLOW}[-] Binary $1 not found — building...${RESET}"
        make -C "$ARSENAL_ROOT" "$bin" 2>&1 | tail -5
    fi
}

module="${1:-help}"
shift || true

case "$module" in
    brute)
        sub="${1:-multi}"; shift || true
        # Map arguments through filter_args
        mapfile -t PARSED_ARGS < <(filter_args "$@")
        case "$sub" in
            zip)   build_if_needed zip_cracker;  "$BIN_DIR/zip_cracker" "${PARSED_ARGS[@]}" ;;
            rar)   build_if_needed rar_cracker;  "$BIN_DIR/rar_cracker" "${PARSED_ARGS[@]}" ;;
            pdf)   build_if_needed pdf_cracker;  "$BIN_DIR/pdf_cracker" "${PARSED_ARGS[@]}" ;;
            multi) build_if_needed multi_brute;  "$BIN_DIR/multi_brute" "${PARSED_ARGS[@]}" ;;
            *) echo -e "${RED}[!] Unknown brute sub-module: $sub${RESET}"; exit 1 ;;
        esac ;;
    rainbow)
        sub="${1:-gen}"; shift || true
        case "$sub" in
            gen)   build_if_needed table_generator; "$BIN_DIR/table_generator" "$@" ;;
            query) build_if_needed table_query;     "$BIN_DIR/table_query" "$@" ;;
            *) echo -e "${RED}[!] Unknown rainbow sub-module: $sub${RESET}"; exit 1 ;;
        esac ;;
    stego)
        python3 "$SRC_DIR/stego/stego_scan.py" "$@" ;;
    clue)
        python3 "$SRC_DIR/clue/clue_scanner.py" "$@" ;;
    fuzz)
        sub="${1:-http}"; shift || true
        case "$sub" in
            http) build_if_needed http_fuzz; "$BIN_DIR/http_fuzz" "$@" ;;
            dns)  build_if_needed dns_fuzz;  "$BIN_DIR/dns_fuzz" "$@" ;;
            smb)  build_if_needed smb_fuzz;  "$BIN_DIR/smb_fuzz" "$@" ;;
            *) echo -e "${RED}[!] Unknown fuzz sub-module: $sub${RESET}"; exit 1 ;;
        esac ;;
    wordlists|lists|wl)
        list_wordlists ;;
    help|--help|-h)
        usage ;;
    *)
        echo -e "${RED}[!] Unknown module: $module${RESET}"
        usage; exit 1 ;;
esac
