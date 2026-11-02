#!/usr/bin/env python3
"""
Deterministic generator for PadTest (3ds_padtest) packaging assets:
- meta/icon.png (48x48 RGBA PNG: Warm Amber/Orange D-Pad Cross + Calibration Target Ring)
- meta/banner.png (256x128 RGBA PNG: D-Pad / Crosshair + short title text)
- meta/audio.wav & meta/banner.bnr
"""

import math
import os
import struct
import wave
import zlib
from generate_bnr_shared import draw_banner_text_centered, write_cbmd_bnr  # type: ignore


def write_png(path: str, width: int, height: int, rgba_bytes: bytes) -> None:
    def chunk(tag: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    raw_rows = bytearray()
    stride = width * 4
    for y in range(height):
        raw_rows.append(0)
        raw_rows.extend(rgba_bytes[y * stride : (y + 1) * stride])

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    compressed = zlib.compress(bytes(raw_rows), 9)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", compressed))
        f.write(chunk(b"IEND", b""))


def generate_icon(path: str) -> None:
    w, h = 48, 48
    buf = bytearray(w * h * 4)
    cx, cy = 23.5, 23.5
    for y in range(h):
        for x in range(w):
            idx = (y * w + x) * 4
            if x < 2 or x >= w - 2 or y < 2 or y >= h - 2:
                buf[idx : idx + 4] = bytes((255, 155, 65, 255))
                continue
            buf[idx : idx + 4] = bytes((20, 18, 28, 255))

            dx = abs(x - cx)
            dy = abs(y - cy)
            r = math.sqrt(dx * dx + dy * dy)

            # Outer calibration circle
            if 16.5 <= r <= 18.2:
                buf[idx : idx + 4] = bytes((95, 210, 255, 255))
            # Classic D-Pad cross in center
            elif (dx <= 4.5 and dy <= 13.5) or (dy <= 4.5 and dx <= 13.5):
                if r <= 3.0:
                    buf[idx : idx + 4] = bytes((255, 235, 120, 255))
                else:
                    buf[idx : idx + 4] = bytes((255, 145, 55, 255))

    write_png(path, w, h, bytes(buf))


def generate_banner(path: str) -> bytes:
    w, h = 256, 128
    buf = bytearray(w * h * 4)
    cx, cy = 128.0, 44.0
    for y in range(h):
        for x in range(w):
            idx = (y * w + x) * 4
            r = 22 + (y * 16) // h
            g = 18 + (y * 14) // h
            b = 30 + (y * 22) // h
            buf[idx : idx + 4] = bytes((r, g, b, 255))

            dx = abs(x - cx)
            dy = abs(y - cy)
            dist = math.sqrt(dx * dx + dy * dy)
            if 26.0 <= dist <= 29.0:
                buf[idx : idx + 4] = bytes((95, 210, 255, 255))
            elif (dx <= 8.0 and dy <= 22.0) or (dy <= 8.0 and dx <= 22.0):
                buf[idx : idx + 4] = bytes((255, 155, 65, 255))

    draw_banner_text_centered(buf, w, h, 86, "PADTEST 3DS", 2, (255, 185, 85, 255))
    draw_banner_text_centered(buf, w, h, 108, "STICK DRIFT & SCREEN TEST", 1, (215, 235, 255, 255))
    raw = bytes(buf)
    write_png(path, w, h, raw)
    return raw


def generate_audio(path: str) -> None:
    sample_rate = 22050
    duration_s = 0.8
    n_samples = int(sample_rate * duration_s)
    with wave.open(path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        frames = bytearray()
        for i in range(n_samples):
            t = i / sample_rate
            freq = 493.88 if t < 0.25 else (659.25 if t < 0.5 else 783.99)
            env = max(0.0, 1.0 - (t / duration_s))
            val = int(12000 * env * math.sin(2.0 * math.pi * freq * t))
            frames.extend(struct.pack("<h", val))
        wf.writeframes(bytes(frames))


def main() -> None:
    meta_dir = os.path.dirname(os.path.abspath(__file__))
    generate_icon(os.path.join(meta_dir, "icon.png"))
    banner_rgba = generate_banner(os.path.join(meta_dir, "banner.png"))
    generate_audio(os.path.join(meta_dir, "audio.wav"))
    write_cbmd_bnr(os.path.join(meta_dir, "banner.bnr"), (493.88, 659.25, 783.99), banner_rgba)
    print("Generated PadTest meta/icon.png, meta/banner.png, meta/audio.wav, meta/banner.bnr")


if __name__ == "__main__":
    main()
