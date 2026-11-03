import os, sys, random, shutil, yaml
from ultralytics import YOLO

root = os.path.abspath(sys.argv[1])
VAL_RATIO = 0.15
EXT = (".jpg", ".jpeg", ".png")
random.seed(0)

tr_img = os.path.join(root, "train", "images")
tr_lbl = os.path.join(root, "train", "labels")
va_img = os.path.join(root, "val", "images")
va_lbl = os.path.join(root, "val", "labels")

# buat split val dari train jika belum ada
if not os.path.isdir(va_img):
    os.makedirs(va_img)
    os.makedirs(va_lbl)
    imgs = sorted(f for f in os.listdir(tr_img) if f.lower().endswith(EXT))
    random.shuffle(imgs)
    for f in imgs[:max(1, int(len(imgs) * VAL_RATIO))]:
        shutil.move(os.path.join(tr_img, f), os.path.join(va_img, f))
        txt = os.path.splitext(f)[0] + ".txt"
        if os.path.exists(os.path.join(tr_lbl, txt)):
            shutil.move(os.path.join(tr_lbl, txt), os.path.join(va_lbl, txt))
    print("Split val dibuat:", len(os.listdir(va_img)), "gambar")

# nama kelas: dari data.yaml jika ada, jika tidak dari ID di label
src_yaml = os.path.join(root, "data.yaml")
if os.path.exists(src_yaml) and yaml.safe_load(open(src_yaml)).get("names"):
    names = yaml.safe_load(open(src_yaml))["names"]
else:
    max_id = 0
    for split in ("train", "val", "test"):
        d = os.path.join(root, split, "labels")
        if not os.path.isdir(d):
            continue
        for f in os.listdir(d):
            for line in open(os.path.join(d, f)).read().split("\n"):
                if line:
                    max_id = max(max_id, int(line.split()[0]))
    names = {i: f"class_{i}" for i in range(max_id + 1)}

cfg = {"path": root, "train": "train/images", "val": "val/images", "names": names}
if os.path.isdir(os.path.join(root, "test", "images")):
    cfg["test"] = "test/images"
data_yaml = os.path.join(root, "data_abs.yaml")
yaml.safe_dump(cfg, open(data_yaml, "w"))
print("Kelas:", names)

model = YOLO("yolo26n.pt")
model.train(
    data=data_yaml,
    imgsz=640,
    batch=16,
    epochs=100,
    device="cuda:0",
    workers=4,
    optimizer="auto",
    patience=20,
    hsv_h=0.1, hsv_s=0.7, hsv_v=0.5,
    degrees=15, scale=0.5, fliplr=0.5,
    project=os.path.join(root, "runs"), name="knight",
)

best = os.path.join(str(model.trainer.save_dir), "weights", "best.pt")
if "test" in cfg:
    m = YOLO(best).val(data=data_yaml, imgsz=640, split="test", device="cuda:0")
    print("TEST mAP50:", m.box.map50, " mAP50-95:", m.box.map)
