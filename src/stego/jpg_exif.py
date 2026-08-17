#!/usr/bin/env python3
"""
jpg_exif.py — JPEG EXIF metadata inspector and injector
Uses Pillow. Reads all EXIF tags and can inject UserComment.
"""
import sys
from PIL import Image
from PIL.ExifTags import TAGS


def dump(src: str):
    """Print all EXIF tags found in a JPEG."""
    img = Image.open(src)
    exif_data = img._getexif()
    if not exif_data:
        print("[-] No EXIF data found")
        return
    print(f"[+] EXIF data in {src}:")
    for tag_id, value in sorted(exif_data.items()):
        tag = TAGS.get(tag_id, f"Tag_{tag_id:#06x}")
        # Truncate very long values
        str_val = str(value)
        if len(str_val) > 120:
            str_val = str_val[:117] + "..."
        print(f"    {tag:<30s}: {str_val}")


def inject(src: str, data: str, dst: str):
    """Inject data into EXIF UserComment field."""
    try:
        import piexif
    except ImportError:
        print("[!] piexif not installed: pip install piexif", file=sys.stderr)
        sys.exit(1)

    img = Image.open(src)
    try:
        exif_dict = piexif.load(img.info.get("exif", b""))
    except Exception:
        exif_dict = {"0th": {}, "Exif": {}, "GPS": {}, "1st": {}}

    # UserComment: prefix with ASCII\x00\x00\x00\x00\x00\x00\x00\x00
    comment = b"ASCII\x00\x00\x00\x00\x00\x00\x00\x00" + data.encode("utf-8")
    exif_dict["Exif"][piexif.ExifIFD.UserComment] = comment
    exif_bytes = piexif.dump(exif_dict)
    img.save(dst, exif=exif_bytes)
    print(f"[+] Injected {len(data)} chars into EXIF UserComment -> {dst}")


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="JPEG EXIF Inspector")
    ap.add_argument("action", choices=["dump", "inject"])
    ap.add_argument("src"); ap.add_argument("--data"); ap.add_argument("--out")
    a = ap.parse_args()
    if a.action == "dump":
        dump(a.src)
    else:
        inject(a.src, a.data, a.out or "out.jpg")
