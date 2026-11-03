#!/usr/bin/env python3
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, numpy as np
from runner import run_node

MIN_AREA_RATIO = 0.002   # area minimum relatif terhadap luas frame
EPS_RATIO = 0.03         # toleransi approxPolyDP terhadap perimeter
CIRCLE_CIRC = 0.92       # circularity minimum untuk lingkaran

NAMES = {3: "Segitiga", 4: "Persegi", 5: "Pentagon", 6: "Heksagon"}
COLORS = {"Segitiga": (0, 0, 255), "Persegi": (0, 255, 0), "Pentagon": (255, 0, 0),
          "Heksagon": (0, 255, 255), "Lingkaran": (255, 0, 255)}

def foreground_mask(frame):
    lab = cv2.cvtColor(cv2.GaussianBlur(frame, (5, 5), 0), cv2.COLOR_BGR2LAB).astype(np.float32)
    border = np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]])
    bg = np.median(border, axis=0)
    dist = np.linalg.norm(lab - bg, axis=2)
    dist = cv2.normalize(dist, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
    _, mask = cv2.threshold(dist, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    k = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, k)
    return cv2.morphologyEx(mask, cv2.MORPH_CLOSE, k)

def classify(cnt):
    area = cv2.contourArea(cnt)
    peri = cv2.arcLength(cnt, True)
    if peri == 0:
        return None
    circ = 4 * np.pi * area / (peri * peri)
    n = len(cv2.approxPolyDP(cnt, EPS_RATIO * peri, True))
    if circ >= CIRCLE_CIRC or n >= 7:
        return "Lingkaran"
    if n == 4:
        x, y, w, h = cv2.boundingRect(cnt)
        return "Persegi" if 0.85 <= w / h <= 1.15 else "Persegi Panjang"
    return NAMES.get(n)

def process(frame):
    out = frame.copy()
    h, w = frame.shape[:2]
    mask = foreground_mask(frame)
    cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    counts = {}
    for c in cnts:
        if cv2.contourArea(c) < MIN_AREA_RATIO * w * h:
            continue
        label = classify(c)
        if label is None:
            continue
        color = COLORS.get(label, (255, 255, 255))
        x, y, bw, bh = cv2.boundingRect(c)
        cv2.drawContours(out, [c], -1, color, 2)
        cv2.rectangle(out, (x, y), (x + bw, y + bh), color, 1)
        cv2.putText(out, label, (x, max(y - 6, 15)), cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
        counts[label] = counts.get(label, 0) + 1
    y0 = 60
    for k, v in counts.items():
        cv2.putText(out, f"{k}: {v}", (10, y0), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        y0 += 25
    return out

if __name__ == "__main__":
    run_node("shapes", process)
