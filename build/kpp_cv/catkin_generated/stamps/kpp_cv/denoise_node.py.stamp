#!/usr/bin/env python3
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, numpy as np
from runner import run_node

NOISE_AREA_RATIO = 0.02   # kontur < 2% dari kontur terbesar dianggap noise
KERNEL_RATIO = 0.004      # ukuran kernel morfologi relatif ke sisi pendek frame
MAX_DISPLAY_W = 1800      # lebar maksimum jendela tampilan

def get_kernel(shape):
    k = max(3, int(min(shape[:2]) * KERNEL_RATIO) | 1)  # selalu ganjil
    return np.ones((k, k), np.uint8)

def object_mask(frame):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, mask = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    if np.count_nonzero(mask) > mask.size // 2:   # objek harus putih, latar hitam
        mask = cv2.bitwise_not(mask)
    return mask

def remove_small(mask):
    cnts, _ = cv2.findContours(mask, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    if not cnts:
        return mask
    areas = [cv2.contourArea(c) for c in cnts]
    limit = NOISE_AREA_RATIO * max(areas)
    for c, a in zip(cnts, areas):
        if a < limit:
            cv2.drawContours(mask, [c], -1, 0, -1)
    return mask

def process(frame):
    k = get_kernel(frame.shape)
    mask = object_mask(frame)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, k)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, k)
    mask = remove_small(mask)

    result = cv2.bitwise_and(frame, frame, mask=mask)
    mask_bgr = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    out = np.concatenate([frame, mask_bgr, result], axis=1)

    if out.shape[1] > MAX_DISPLAY_W:
        s = MAX_DISPLAY_W / out.shape[1]
        out = cv2.resize(out, (int(out.shape[1] * s), int(out.shape[0] * s)))
    pw = out.shape[1] // 3
    for i, t in enumerate(["Asli", "Mask bersih", "Hasil"]):
        cv2.putText(out, t, (i * pw + 10, out.shape[0] - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
    return out

if __name__ == "__main__":
    run_node("denoise", process)
