#!/usr/bin/env bash
# docker_build.sh — Build CTF Arsenal in a Docker container
# Produces a portable image with all tools pre-compiled.
set -euo pipefail

IMAGE_NAME="ctf-arsenal:latest"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

echo "[*] Building CTF Arsenal Docker image: $IMAGE_NAME"

cat > /tmp/ctf_arsenal.Dockerfile << 'EOF'
FROM archlinux:latest
RUN pacman -Sy --needed --noconfirm base-devel openssl libzip libarchive libpcap poppler python python-pillow python-numpy unrar && pip install piexif --break-system-packages
WORKDIR /ctf_arsenal
COPY . .
RUN make all
ENTRYPOINT ["/ctf_arsenal/scripts/ctf_arsenal.sh"]
EOF

docker build -f /tmp/ctf_arsenal.Dockerfile -t "$IMAGE_NAME" "$REPO_ROOT"
echo "[+] Docker image built: $IMAGE_NAME"
echo "[*] Usage: docker run --rm -it $IMAGE_NAME help"
