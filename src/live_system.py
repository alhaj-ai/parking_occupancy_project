import os
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"
import sys
ROOT_DIR = os.path.dirname(os.path.dirname(__file__))
sys.path.append(ROOT_DIR)
import cv2
import serial
import time
from ultralytics import YOLO
from config.parking_spots import PARKING_SPOTS

# ====== إعداد Serial ======
try:
    esp = serial.Serial("COM10", 115200, timeout=1)
    time.sleep(2)
    print("✅ تم الاتصال مع ESP32")
except:
    esp = None
    print("⚠️ لم يتم الاتصال مع ESP (وضع اختبار)")

# ====== تحميل المودل ======
model = YOLO("runs/classify/train/weights/best.pt")

CONF_THRESHOLD = 0.7

cap = cv2.VideoCapture(1)

print("اضغط ESC للخروج")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    parking_states = []

    for i, (x1, y1, x2, y2) in enumerate(PARKING_SPOTS):
        crop = frame[y1:y2, x1:x2]

        if crop.size == 0:
            parking_states.append(1)
            continue

        results = model(crop, verbose=False)[0]
        probs = results.probs.data.cpu().numpy()

        class_id = probs.argmax()
        confidence = probs[class_id]
        label = model.names[class_id]

        occupied = 1 if (label == "occupied" and confidence >= CONF_THRESHOLD) else 0
        parking_states.append(occupied)

        color = (0, 0, 255) if occupied else (0, 255, 0)
        cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)

        cv2.putText(
            frame,
            f"P{i+1}:{label}",
            (x1, y1 - 5),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            color,
            1
        )

    # ====== إرسال البيانات للـ ESP ======
    msg = ",".join(map(str, parking_states)) + "\n"
    print("Send:", msg.strip())

    if esp:
        esp.write(msg.encode())

    cv2.imshow("Smart Parking", frame)

    if cv2.waitKey(1) == 27:
        break

cap.release()
cv2.destroyAllWindows()
