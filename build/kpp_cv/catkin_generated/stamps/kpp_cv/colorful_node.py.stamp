#!/usr/bin/env python3
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, numpy as np
from runner import run_node

SHIFT = 5                 # 256 >> 5 = 8 level per kanal (512 warna)
MIN_SHARE = 0.005         # porsi piksel minimum agar dihitung warna unik
MAX_SWATCH = 12           # jumlah kotak palet yang digambar
DIM = 0.25                # kecerahan area non-dominan

L = 256 >> SHIFT
HALF = 1 << (SHIFT - 1)

def quantize_key(frame):
    q = (frame >> SHIFT).astype(np.int32)
    return (q[..., 0] * L + q[..., 1]) * L + q[..., 2]

def key_to_bgr(key):
    r = key % L
    g = (key // L) % L
    b = key // (L * L)
    return tuple(int((v << SHIFT) + HALF) for v in (b, g, r))

def process(frame):
    h, w = frame.shape[:2]
    key = quantize_key(frame)
    counts = np.bincount(key.ravel(), minlength=L ** 3)
    total = key.size

    dom = int(counts.argmax())
    mask = key == dom
    out = (frame * DIM).astype(np.uint8)
    out[mask] = frame[mask]

    valid = np.where(counts >= MIN_SHARE * total)[0]
    valid = valid[np.argsort(-counts[valid])]

    # palet warna unik di bagian bawah
    sw = max(w // MAX_SWATCH, 1)
    sh = 40
    for i, k in enumerate(valid[:MAX_SWATCH]):
        color = key_to_bgr(int(k))
        x = i * sw
        cv2.rectangle(out, (x, h - sh), (x + sw, h), color, -1)
        cv2.putText(out, f"{100 * counts[k] / total:.1f}%", (x + 4, h - 14),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)

    b, g, r = key_to_bgr(dom)
    cv2.rectangle(out, (10, 45), (40, 75), (b, g, r), -1)
    cv2.putText(out, f"Dominan #{r:02X}{g:02X}{b:02X} ({100 * counts[dom] / total:.1f}%)",
                (50, 68), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    cv2.putText(out, f"Warna unik: {len(valid)}", (10, 100),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    return out

if __name__ == "__main__":
    run_node("colorful", process)
