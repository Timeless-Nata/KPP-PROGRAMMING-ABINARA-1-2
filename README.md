# KPP Object Detection – Computer Vision dengan ROS Noetic

Lima studi kasus computer vision yang berjalan **real-time** di atas **ROS Noetic** (Python). Satu node *publisher* membaca video dan mengirim frame ke topic `/camera/image_raw`. Setiap studi kasus adalah satu node *subscriber* yang memproses frame secara dinamis, **tanpa hardcoding** posisi, koordinat, jenis objek, maupun timing berdasarkan nomor frame atau detik.

| No | Studi kasus | Pendekatan | Launch file |
|----|-------------|-----------|-------------|
| 1 | Where Are The Shapes? | Pemrosesan citra klasik | `shapes.launch` |
| 2 | How Colorful? | Pemrosesan citra klasik | `colorful.launch` |
| 3 | Locate It, Please. | Pemrosesan citra klasik | `locate.launch` |
| 4 | I Don't Think It's Reliable. | Pemrosesan citra klasik | `denoise.launch` |
| 5 | Does It Match? | YOLO26n (hasil training sendiri) | `match.launch` |

## Struktur repositori

```
kpp-magang-5/
└── src/kpp_cv/
    ├── CMakeLists.txt, package.xml
    ├── launch/        # satu launch file per studi kasus (+ test.launch)
    ├── scripts/
    │   ├── publisher_node.py   # video -> topic (generik, dipakai semua soal)
    │   ├── runner.py           # kerangka subscriber + overlay FPS
    │   ├── shapes_node.py
    │   ├── colorful_node.py
    │   ├── locate_node.py
    │   ├── denoise_node.py
    │   ├── match_node.py
    │   └── viewer_node.py      # uji tampilan video
    ├── tools/
    │   ├── check_dataset.py    # pemeriksaan dataset YOLO
    │   └── train.py            # training YOLO26n (soal 5)
    ├── models/best.pt          # bobot hasil training soal 5
    └── data/                   # video dan dataset (tidak ikut repo)
```

## Lingkungan

| Komponen | Keterangan |
|----------|-----------|
| Lingkungan ROS | Distrobox Ubuntu 20.04, ROS Noetic, Python 3.8 |
| Training (soal 5) | Di host, venv Python 3.12 (uv), PyTorch CUDA 12.8, ultralytics 8.4.x |
| Inferensi (soal 5) | Di container ROS, CPU |

Training dilakukan di host karena GPU Blackwell membutuhkan PyTorch CUDA 12.8 ke atas yang tidak tersedia untuk Python 3.8. Hasilnya (`best.pt`) lalu dipakai oleh node ROS di container.

### Instalasi di container ROS

```bash
sudo apt install ros-noetic-desktop-full
mkdir -p ~/kpp-magang-5/src && cd ~/kpp-magang-5
# letakkan folder kpp_cv di src/, lalu:
catkin_make
source devel/setup.bash

# khusus soal 5 (gunakan /usr/bin/python3 = Python 3.8 sistem, bukan venv)
/usr/bin/python3 -m pip install --user "numpy<2"
/usr/bin/python3 -m pip install --user torch torchvision --index-url https://download.pytorch.org/whl/cpu
/usr/bin/python3 -m pip install --user ultralytics
```

> **Peringatan:** jangan memasang `opencv-python` atau `opencv-python-headless` lewat pip di Python 3.8. Paket itu menimpa OpenCV bawaan apt dan merusak `cv_bridge` (`KeyError` saat konversi frame) atau menghilangkan `cv2.imshow`. Jika sudah terlanjur: `python3 -m pip uninstall -y opencv-python opencv-python-headless`.

## Cara menjalankan

Setiap soal dijalankan dengan satu perintah. Tekan `ESC` pada jendela atau `Ctrl-C` pada terminal untuk berhenti. Overlay **FPS** tampil di pojok kiri atas.

```bash
cd ~/kpp-magang-5 && source devel/setup.bash

roslaunch kpp_cv shapes.launch   video:="/path/ke/video_shapes.mp4"
roslaunch kpp_cv colorful.launch video:="/path/ke/video_colorful.mp4"
roslaunch kpp_cv locate.launch   video:="/path/ke/video_locate.mp4"
roslaunch kpp_cv denoise.launch  video:="/path/ke/video_denoise.mp4"
roslaunch kpp_cv match.launch    video:="/path/ke/gameplay.mp4"
```

Argumen tambahan untuk soal 5: `conf:=0.5` (ambang kepercayaan), `device:=cpu`, `target:=` (filter nama atau nomor kelas, kosong berarti semua), `model:=` (path bobot lain).

## Metode per studi kasus

Semua parameter di bawah relatif terhadap ukuran frame atau terhadap isi frame itu sendiri, tidak pernah terhadap nomor frame.

### 1. Where Are The Shapes?
<img width="1183" height="255" alt="image" src="https://github.com/user-attachments/assets/6be6f742-1b26-46f6-9fa0-abfb36b4fcbb" />

1. Warna latar diestimasi dari **median piksel tepi frame**, lalu jarak tiap piksel ke warna latar dihitung di ruang warna LAB. Cara ini tidak bergantung pada warna bentuk maupun latar.
2. **Otsu threshold** pada peta jarak, lalu morfologi *opening* dan *closing* (kernel elips 5×5).
3. `findContours` (hanya kontur luar), kontur dengan luas di bawah 0,2% luas frame dibuang.
4. Klasifikasi dengan `approxPolyDP` (toleransi 3% dari perimeter): 3 sisi segitiga, 4 sisi persegi (rasio lebar/tinggi 0,85 sampai 1,15), 5 sisi pentagon, 6 sisi heksagon. **Lingkaran** ditentukan dari *circularity* `4πA/P² ≥ 0,92` atau jumlah sisi hampir ≥ 7.

### 2. How Colorful?
<img width="1366" height="229" alt="image" src="https://github.com/user-attachments/assets/86b6ab86-bc36-4afe-8ca5-275de85d2cb2" />

1. Warna **dikuantisasi** per kanal (8 level per kanal, 512 kemungkinan warna) agar anti-aliasing dan kompresi tidak menghasilkan ribuan "warna unik" palsu.
2. Kemunculan dihitung **per piksel** dengan `np.bincount` pada seluruh frame (bukan per region).
3. Sebuah warna dianggap unik jika porsinya minimal 0,5% dari seluruh piksel.
4. Warna dominan (piksel terbanyak) disegmentasi dengan mask, area lain digelapkan. Palet semua warna unik beserta persentasenya digambar di bagian bawah frame.

### 3. Locate It, Please.
<img width="1366" height="229" alt="image" src="https://github.com/user-attachments/assets/14ffb6ba-2bad-4e73-88f0-b3d928200b10" />

Mendeteksi semua objek (berlian yang berjatuhan) tanpa klasifikasi.
1. Frame diperkecil ke lebar kerja 960 px agar tetap real-time. Warna latar = **median seluruh frame**, lalu jarak warna LAB per piksel.
2. **Hysteresis threshold**: ambang kuat dari Otsu, ambang lemah 0,4× ambang kuat. Hanya komponen lemah yang memuat piksel kuat yang dipertahankan, sehingga seluruh badan berlian tertangkap dan bukan hanya kilauannya.
3. Tepi **Canny** di sekitar objek ditambahkan, lalu *closing* dan pengisian lubang kontur.
4. Objek yang bersinggungan dipisah dengan **distance transform + watershed**. Inti tiap objek dicari pada beberapa level rasio (0,7 sampai 0,3) dan dipilih yang menghasilkan inti terbanyak. Jika gagal dipecah, blob tetap mendapat satu bounding box.

### 4. I Don't Think It's Reliable.
<img width="1435" height="213" alt="image" src="https://github.com/user-attachments/assets/bfdff1be-185d-48e1-8d65-e170246fe239" />

1. Grayscale lalu **Otsu threshold**. Mask dibalik otomatis jika piksel putih mendominasi, sehingga objek selalu putih.
2. *Opening* menghapus bintik noise dan *closing* menutup lubang kecil (kernel 0,4% sisi pendek frame).
3. `findContours` + `contourArea`: kontur dengan luas di bawah 2% dari kontur terbesar dianggap noise dan dihapus. Lubang pada objek (misalnya lubang donat) tetap utuh.
4. Mask bersih diterapkan ke frame dengan `bitwise_and`. Tampilan terdiri dari tiga panel: asli, mask bersih, hasil.

### 5. Does It Match?
<img width="1226" height="180" alt="image" src="https://github.com/user-attachments/assets/9a743a84-7aaf-4575-a758-8eb958f856c8" />

Mendeteksi karakter **Knight** (Hollow Knight) pada video gameplay, dengan variasi warna, lighting, dan transformasi.
- **Model:** YOLO26n, dilatih dengan `tools/train.py` pada dataset yang disediakan (597 gambar train, 75 gambar test, satu kelas).
- **Split validasi:** 15% dari data train (89 gambar) dipisahkan dengan seed tetap, sehingga train 508 gambar, val 89, dan test 75. Test tidak dipakai saat training.
- **Hyperparameter:** `imgsz=640`, `batch=16`, maksimal 100 epoch, `patience=20` (early stopping), `workers=4`.
- **Augmentasi:** `hsv_h=0.1`, `hsv_s=0.7`, `hsv_v=0.5` (variasi warna dan pencahayaan), `degrees=15`, `scale=0.5`, `fliplr=0.5`.
- **Training berhenti di epoch 64** karena early stopping, dan bobot terbaik disimpan di `models/best.pt`.
- **Inferensi:** `match_node.py` memuat model sekali, lalu menjalankan deteksi per frame dengan ambang `conf=0.5` dan menggambar kotak serta skor.

Reproduksi training (di host dengan GPU):
```bash
python tools/check_dataset.py data/5_does_it_match
python tools/train.py data/5_does_it_match
```

## Sumber dan atribusi

- Video dan dataset disediakan oleh panitia KPP. Video soal 1 dan 2 berasal dari materi soal (animasi 2veinte untuk soal 2), soal 3 dari Pixabay (Vimeo-Free-Videos), soal 4 dari GettyImages.
- Dataset soal 5 (Hollow Knight, format YOLO) disediakan panitia, dengan sumber Roboflow.
- Library: ROS Noetic, OpenCV, NumPy, PyTorch, Ultralytics.
- **Penggunaan AI:** Generative AI dipakai sebagai alat bantu pengembangan sesuai ketentuan KPP. Seluruh kode telah dipelajari dan diuji oleh penulis.
