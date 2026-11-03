#!/usr/bin/env python3
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, numpy as np
from runner import run_node

SCALE_W = 640            # lebar kerja (frame dikecilkan agar cepat)
MIN_AREA_RATIO = 0.0003  # luas minimum objek relatif terhadap luas frame
MIN_DIST = 25            # jarak warna minimum dari latar (unit LAB)
KERNEL_RATIO = 0.006     # kernel morfologi relatif ke sisi pendek frame

def get_kernel(shape):
    k = max(3, int(min(shape[:2]) * KERNEL_RATIO) | 1)
    return np.ones((k, k), np.uint8)

def object_mask(small):
    lab = cv2.cvtColor(cv2.GaussianBlur(small, (5, 5), 0), cv2.COLOR_BGR2LAB).astype(np.float32)
    bg = np.median(lab[::4, ::4].reshape(-1, 3), axis=0)   # warna latar = warna paling umum
    dist = np.clip(np.linalg.norm(lab - bg, axis=2), 0, 255).astype(np.uint8)
    t, _ = cv2.threshold(dist, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    _, mask = cv2.threshold(dist, max(t, MIN_DIST), 255, cv2.THRESH_BINARY)
    k = get_kernel(small.shape)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, k)
    return cv2.morphologyEx(mask, cv2.MORPH_CLOSE, k)

def process(frame):
    h, w = frame.shape[:2]
    s = min(1.0, SCALE_W / w)
    small = cv2.resize(frame, (int(w * s), int(h * s))) if s < 1.0 else frame
    mask = object_mask(small)

    cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    min_area = MIN_AREA_RATIO * small.shape[0] * small.shape[1]
    out = frame.copy()
    n = 0
    for c in cnts:
        if cv2.contourArea(c) < min_area:
            continue
        x, y, bw, bh = cv2.boundingRect(c)
        x, y, bw, bh = int(x / s), int(y / s), int(bw / s), int(bh / s)
        cv2.rectangle(out, (x, y), (x + bw, y + bh), (0, 255, 0), 2)
        n += 1
    cv2.putText(out, f"Objek: {n}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    return out

if __name__ == "__main__":
    run_node("locate", process)
