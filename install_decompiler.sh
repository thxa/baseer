#!/usr/bin/env bash
set -e

URL="https://github.com/thxa/baseer/releases/download/v0.2.0/decompiler.tar.xz"
FILE="decompiler.tar.xz"
DEST="/opt/baseer"

echo "[*] Downloading decompiler..."
curl -L --progress-bar -o "$FILE" "$URL"

echo "[*] Creating target directory..."
mkdir -p "$DEST"

echo "[*] Extracting..."
tar xf "$FILE" -C "$DEST"

echo "[*] Cleaning up..."
rm "$FILE"

echo "[+] Decompiler installed successfully!"

