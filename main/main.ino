#include <ESP32Servo.h>  // مكتبة السرفو للـ ESP32

// ====== Parking LEDs Pins ======
int redPins[4]   = {23, 22, 21, 19};
int greenPins[4] = {14, 12, 32, 33};

// ====== Servo ======
const int SERVO_PIN = 27;         // الموصوف من عندك
Servo gateServo;
const int OPEN_ANGLE   = 180;      // زاوية "مفتوح" (اضبط حسب حاجتك)
const int CLOSED_ANGLE = 0;       // زاوية "مغلق" (اضبط حسب حاجتك)

bool servoIsClosed = false;       // الحالة الحالية للسرفو

void setup() {
  Serial.begin(9600);

  // إعداد المخرج لللمبات
  for (int i = 0; i < 4; i++) {
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    digitalWrite(redPins[i], LOW);
    digitalWrite(greenPins[i], LOW);
  }

  // إعداد السرفو
  gateServo.attach(SERVO_PIN);
  gateServo.write(OPEN_ANGLE); // افتراضيا افتح البوابة لأن على الأقل مكان واحد قد يكون متوفر
  servoIsClosed = false;

  Serial.println("ESP32 READY");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();

    // نتوقع الشكل: "0,1,0,1"
    int states[4] = {1,1,1,1}; // قيمة افتراضية: ممتلئ
    int parsed = sscanf(data.c_str(), "%d,%d,%d,%d",
                        &states[0], &states[1], &states[2], &states[3]);

    // لو لم تتم القراءة بشكل صحيح نتجاهل السطر
    if (parsed < 4) {
      Serial.print("Bad line: ");
      Serial.println(data);
      return;
    }

    // تحديث اللمبات
    for (int i = 0; i < 4; i++) {
      if (states[i] == 1) {
        digitalWrite(redPins[i], HIGH);
        digitalWrite(greenPins[i], LOW);
      } else {
        digitalWrite(redPins[i], LOW);
        digitalWrite(greenPins[i], HIGH);
      }
    }

    // هل كل المواقف حمراء؟
    bool allRed = true;
    for (int i = 0; i < 4; i++) {
      if (states[i] == 0) { allRed = false; break; }
    }

    // منطق السرفو: إذا كلهم حمر -> أغلق؛ وإلّا افتح
    if (allRed && !servoIsClosed) {
      gateServo.write(CLOSED_ANGLE);
      servoIsClosed = true;
      Serial.println("Servo -> CLOSED");
    } else if (!allRed && servoIsClosed) {
      gateServo.write(OPEN_ANGLE);
      servoIsClosed = false;
      Serial.println("Servo -> OPEN");
    }

    // لوج للتأكد
    Serial.print("States: ");
    Serial.print(states[0]); Serial.print(",");
    Serial.print(states[1]); Serial.print(",");
    Serial.print(states[2]); Serial.print(",");
    Serial.println(states[3]);
  }
}
