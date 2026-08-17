#!/usr/bin/env bash
# install_deps.sh — Install CTF Arsenal dependencies on Arch/Manjaro
set -euo pipefail

echo "[*] CTF Arsenal — Dependency Installer (Arch/Manjaro)"

PKGS=(
    base-devel          # gcc, make, etc.
    openssl             # -lssl -lcrypto
    libzip              # -lzip
    libarchive          # -larchive
    libpcap             # -lpcap
    poppler             # pdfinfo
    unrar               # rar_cracker subprocess
    python              # Python 3
    python-pillow       # Stego PNG/JPEG
    python-numpy        # Numerical ops
    python-cryptography # Crypto
)

echo "[*] Installing: ${PKGS[*]}"
sudo pacman -Sy --needed --noconfirm "${PKGS[@]}"

# piexif (not in official repos, use pip)
echo "[*] Installing piexif via pip..."
pip install --user piexif 2>/dev/null || echo "  [!] piexif install failed (optional)"

echo "[+] All dependencies installed."
