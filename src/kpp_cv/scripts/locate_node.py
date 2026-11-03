#!/usr/bin/env python3

import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, numpy as np
from runner import run_node

SCALE_W = 960             # lebar kerja

MIN_AREA_RATIO = 0.00002  # luas minimum objek relatif terhadap luas frame

MIN_HIGH = 25             # batas bawah threshold "kuat" (hysteresis)

LOW_FACTOR = 0.4          # threshold "lemah" = LOW_FACTOR * threshold kuat

MIN_LOW = 12              # batas bawah threshold lemah

GROW_RATIO = 0.01         # jangkauan tepi Canny di sekitar objek

CLOSE_RATIO = 0.012       # kernel closing relatif ke sisi pendek frame

LEVELS = (0.7, 0.6, 0.5, 0.4, 0.3)  # level rasio untuk mencari inti berlian

MIN_CORE_PX = 4           # ukuran minimum inti (piksel, pada skala kerja)

DEBUG = False             # True = tampilkan mask kecil di pojok kanan atas


def odd_kernel(shape, ratio):
    k = max(3, int(min(shape[:2]) * ratio) | 1)
    return np.ones((k, k), np.uint8)

def hysteresis_mask(dist):
    t, _ = cv2.threshold(dist, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    high = max(t, MIN_HIGH)
    low = max(LOW_FACTOR * high, MIN_LOW)
    _, m_hi = cv2.threshold(dist, high, 255, cv2.THRESH_BINARY)
    _, m_lo = cv2.threshold(dist, low, 255, cv2.THRESH_BINARY)
    _, labels = cv2.connectedComponents(m_lo)
    keep = np.unique(labels[m_hi > 0])
    keep = keep[keep > 0]
    return np.where(np.isin(labels, keep), 255, 0).astype(np.uint8)

def build_mask(small):
    blur = cv2.GaussianBlur(small, (3, 3), 0)
    lab = cv2.cvtColor(blur, cv2.COLOR_BGR2LAB).astype(np.float32)
    bg = np.median(lab[::4, ::4].reshape(-1, 3), axis=0)
    dist = np.clip(np.linalg.norm(lab - bg, axis=2), 0, 255).astype(np.uint8)
    mask = hysteresis_mask(dist)

    edges = cv2.Canny(cv2.cvtColor(blur, cv2.COLOR_BGR2GRAY), 50, 150)
    grow = cv2.dilate(mask, odd_kernel(small.shape, GROW_RATIO))
    mask = cv2.bitwise_or(mask, cv2.bitwise_and(edges, grow))

    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, odd_kernel(small.shape, CLOSE_RATIO))
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, np.ones((3, 3), np.uint8))

    cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    filled = np.zeros_like(mask)
    cv2.drawContours(filled, cnts, -1, 255, -1)
    return filled

def find_cores(d, inside):
    """Coba beberapa level rasio, pilih yang menghasilkan inti terbanyak."""
    dmax = d.max()
    best, best_n = None, 0
    for r in LEVELS:
        core = ((d >= r * dmax) & inside).astype(np.uint8)
        m, cl = cv2.connectedComponents(core)
        keep = [j for j in range(1, m) if np.count_nonzero(cl == j) >= MIN_CORE_PX]
        if len(keep) > best_n:          # hanya menang jika lebih banyak (level tinggi diutamakan)

            best_n, best = len(keep), (cl, keep)
    return best

def split_and_box(small, mask, min_area):
    n, labels, stats, _ = cv2.connectedComponentsWithStats(mask)
    dist = cv2.distanceTransform(mask, cv2.DIST_L2, 3)
    dist = cv2.GaussianBlur(dist, (3, 3), 0)

    markers = np.zeros(mask.shape, np.int32)
    markers[mask == 0] = 1
    nxt = 2
    comps = []
    for i in range(1, n):
        x, y, w, h, a = stats[i]
        sub = markers[y:y + h, x:x + w]
        inside = labels[y:y + h, x:x + w] == i
        if a < min_area:
            sub[inside] = 1
            continue
        cores = find_cores(dist[y:y + h, x:x + w] * inside, inside)
        k0 = nxt
        if cores is not None:
            cl, keep = cores
            for j in keep:
                sub[cl == j] = nxt
                nxt += 1
        comps.append((x, y, w, h, k0, nxt))

    cv2.watershed(small, markers)

    boxes = []
    for (x, y, w, h, k0, k1) in comps:
        sub = markers[y:y + h, x:x + w]
        found = 0
        for k in range(k0, k1):
            ys, xs = np.nonzero(sub == k)
            if len(xs) == 0:
                continue
            boxes.append((x + xs.min(), y + ys.min(),
                          xs.max() - xs.min() + 1, ys.max() - ys.min() + 1))
            found += 1
        if found == 0:                  # pengaman: blob tidak boleh hilang

            boxes.append((x, y, w, h))
    return boxes

def process(frame):
    h, w = frame.shape[:2]
    s = min(1.0, SCALE_W / w)
    small = cv2.resize(frame, (int(w * s), int(h * s))) if s < 1.0 else frame
    mask = build_mask(small)
    min_area = MIN_AREA_RATIO * small.shape[0] * small.shape[1]
    boxes = split_and_box(small, mask, min_area)

    out = frame.copy()
    for (x, y, bw, bh) in boxes:
        x, y, bw, bh = int(x / s), int(y / s), int(bw / s), int(bh / s)
        cv2.rectangle(out, (x, y), (x + bw, y + bh), (0, 255, 0), 2)
    cv2.putText(out, f"Objek: {len(boxes)}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    if DEBUG:
        mw = w // 4
        mh = int(mask.shape[0] * mw / mask.shape[1])
        thumb = cv2.cvtColor(cv2.resize(mask, (mw, mh)), cv2.COLOR_GRAY2BGR)
        out[0:mh, w - mw:w] = thumb
    return out

if __name__ == "__main__":
    run_node("locate", process)
