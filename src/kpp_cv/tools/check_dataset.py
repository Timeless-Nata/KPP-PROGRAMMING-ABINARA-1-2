import os, sys, yaml

root = os.path.abspath(sys.argv[1])
yaml_path = os.path.join(root, "data.yaml")
if os.path.exists(yaml_path):
    print("Kelas (data.yaml):", yaml.safe_load(open(yaml_path)).get("names"))
else:
    print("data.yaml tidak ada -> kelas dikenali dari ID di label")

for split in ("train", "val", "valid", "test"):
    img_dir = os.path.join(root, split, "images")
    lbl_dir = os.path.join(root, split, "labels")
    if not os.path.isdir(img_dir):
        continue
    imgs = [f for f in os.listdir(img_dir) if f.lower().endswith((".jpg", ".jpeg", ".png"))]
    no_label, empty, bad, per_class = 0, 0, 0, {}
    for f in imgs:
        txt = os.path.join(lbl_dir, os.path.splitext(f)[0] + ".txt")
        if not os.path.exists(txt):
            no_label += 1
            continue
        lines = [l for l in open(txt).read().split("\n") if l]
        if not lines:
            empty += 1
        for line in lines:
            parts = line.split()
            if len(parts) != 5:          # bukan format [cls cx cy w h] (mungkin polygon)
                bad += 1
            cls_id = int(parts[0])
            per_class[cls_id] = per_class.get(cls_id, 0) + 1
    print(f"[{split}] gambar={len(imgs)} tanpa_label={no_label} label_kosong={empty} "
          f"baris_bukan_bbox={bad} kotak_per_kelas={dict(sorted(per_class.items()))}")
