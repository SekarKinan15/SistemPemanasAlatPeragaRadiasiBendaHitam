#include <OneWire.h>
#include <DallasTemperature.h>
#include <TM1637Display.h>


#define DS_PIN 4            // DS18B20
#define RELAY_PIN 5         // Relay active LOW

#define CLK 2               // TM1637
#define DIO 3
TM1637Display display(CLK, DIO);

// ======================
// DS18B20 SETUP
// ======================
OneWire oneWire(DS_PIN);
DallasTemperature sensors(&oneWire);


const uint8_t DEGREE_SYM = 0b01100011;   // °
const uint8_t C_SYM      = 0b00111001;   // C

void setup() {
  Serial.begin(9600);
  sensors.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);    // relay OFF (active LOW)

  display.setBrightness(0x0f);      // max brightness
}

void loop() {

  // ===== Baca Suhu =====
  sensors.requestTemperatures();
  float suhu = sensors.getTempCByIndex(0);

  if (suhu == DEVICE_DISCONNECTED_C) return;

  // tampilkan suhu max 50
  int tampil = (int)suhu;
  if (tampil > 50) tampil = 50;

  // ===== HYSTERESIS SIMPLE =====
  if (suhu < 45) {
    digitalWrite(RELAY_PIN, LOW);   // ON
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // OFF
  }

  // ===== Tampilkan ke TM1637 =====
  int puluhan = tampil / 10;
  int satuan  = tampil % 10;

  uint8_t data[4] = {
    display.encodeDigit(puluhan),
    display.encodeDigit(satuan),
    DEGREE_SYM,
    C_SYM
  };

  display.setSegments(data);

  // ===== Serial Monitor =====
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print("  | Relay: ");
  Serial.println((digitalRead(RELAY_PIN) == LOW) ? "ON" : "OFF");

  delay(300);
}
