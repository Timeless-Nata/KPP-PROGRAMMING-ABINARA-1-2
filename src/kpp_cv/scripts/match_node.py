#!/usr/bin/env python3
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cv2, rospy
from ultralytics import YOLO
from runner import run_node

_state = {}

def load():
    _state["model"] = YOLO(rospy.get_param("~model"))
    _state["conf"] = float(rospy.get_param("~conf", 0.5))
    _state["imgsz"] = int(rospy.get_param("~imgsz", 640))
    _state["device"] = rospy.get_param("~device", "cpu")
    target = str(rospy.get_param("~target", ""))
    _state["target"] = [t.strip().lower() for t in target.split(",") if t.strip()]

def process(frame):
    if "model" not in _state:
        load()
    model = _state["model"]
    outputs = model(frame, conf=_state["conf"], imgsz=_state["imgsz"],
                    device=_state["device"], verbose=False)
    out = frame.copy()
    n = 0
    for output in outputs:
        for det in output.boxes:
            cid = str(int(det.cls[0]))
            name = model.names[int(det.cls[0])]
            tg = _state["target"]
            if tg and name.lower() not in tg and cid not in tg:
                continue
            x1, y1, x2, y2 = [int(v) for v in det.xyxy[0].tolist()]
            score = float(det.conf[0])
            cv2.rectangle(out, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(out, f"{name} {score:.2f}", (x1, max(y1 - 6, 15)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
            n += 1
    cv2.putText(out, f"Cocok: {n}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    return out

if __name__ == "__main__":
    run_node("match", process)
