#include <TM1637Display.h>
#include <math.h>

// ---------------- PIN ----------------
#define PIN_RELAY 4      // input relay di D4 (aktif LOW, NO)
#define PIN_CLK   2      // TM1637 CLK di D2
#define PIN_DIO   3      // TM1637 DIO di D3
#define PIN_NTC   A0     // NTC dibaca di A0

TM1637Display display(PIN_CLK, PIN_DIO);

// ---------------- KONSTANTA NTC ----------------
// Divider: 5V -- Rref(10k) --(A0)-- NTC -- GND
const float R_REF = 10000.0;  // resistor referensi 10k
const float R0    = 10000.0;  // NTC 10k pada 25C
const float BETA  = 3950.0;   // Beta NTC (umum)
const float T0_K  = 298.15;   // 25C dalam Kelvin

// ---------------- SETPOINT & HYSTERESIS ----------------
const float SETPOINT_C   = 50.0;
const float HYSTERESIS_C = 1;

// ---------------- SEGMENT MAP untuk "°C" ----------------
// Segment bit order pada TM1637Display: 0b0GFEDCBA
// Derajat (°) biasa digambar dengan segmen A + B + F + G
// Huruf C digambar dengan segmen A + D + E + F
const uint8_t SEG_DEGREE = 0b01100011; // simbol °
const uint8_t SEG_CHAR_C = 0b00111001; // huruf C

float readNTC_C(int analogPin) {
  int adc = analogRead(analogPin);
  if (adc <= 0) adc = 1;
  if (adc >= 1023) adc = 1022;

  float v = adc / 1023.0;
  float r_ntc = R_REF * (v / (1.0 - v));

  float tempK = 1.0 / ((1.0 / T0_K) + (1.0 / BETA) * log(r_ntc / R0));
  return tempK - 273.15;
}

inline void relayHeaterOn()  { digitalWrite(PIN_RELAY, LOW);  } // aktif LOW
inline void relayHeaterOff() { digitalWrite(PIN_RELAY, HIGH); }

void showTempDegC(int tempRounded) {
  // Batasi ke 0..99 agar rapi 2 digit (karena target 50'C)
  if (tempRounded < 0)  tempRounded = 0;
  if (tempRounded > 99) tempRounded = 99;

  int tens = tempRounded / 10;
  int ones = tempRounded % 10;

  uint8_t segs[4];
  segs[0] = display.encodeDigit(tens);
  segs[1] = display.encodeDigit(ones);
  segs[2] = SEG_DEGREE;
  segs[3] = SEG_CHAR_C;


  // colon bit pada TM1637 dimatikan
  display.setSegments(segs);
}

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  relayHeaterOff();                 // aman saat boot

  display.setBrightness(5);         // 0..7
  display.clear();                  // bersihkan tampilan

  Serial.begin(9600);
  //Serial.println("Sistem Pemanas: NTC 10k + TM1637 (88:88) tampil XX°C, bit colon OFF");
  Serial.println("time_ms,T_bottom_C");

}

void loop() {
  float tempC = readNTC_C(PIN_NTC);
  int tempRounded = (int)lround(tempC); // dibulatkan tanpa desimal

  // Kontrol ON/OFF dengan hysteresis
  if (tempC <= (SETPOINT_C - HYSTERESIS_C)) {
    relayHeaterOn();
  } else if (tempC >= SETPOINT_C) {
    relayHeaterOff();
  }

  // Tampilkan "XX°C"
  showTempDegC(tempRounded);

  // Debug serial
  /*Serial.print("Temp = ");
  Serial.print(tempC, 2);
  Serial.print(" C (rounded ");
  Serial.print(tempRounded);
  Serial.print(") | Relay = ");
  Serial.println(digitalRead(PIN_RELAY) == LOW ? "ON" : "OFF");*/

  // ===== LOG DATA SUHU BAWAH (CSV) =====
  Serial.print(millis());
  Serial.print(",");
  Serial.println(tempC, 2);

  delay(200);
}
