#include<LiquidCrystal.h>

// LCD Pins (RS, EN, D4, D5, D6, D7)
const int rs = 23, en = 22, d4 = 21, d5 = 19, d6 = 18, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pin Definitions
#define TRIG_PIN     4   // Ultrasonic Trig Pin
#define ECHO_PIN     32  // Ultrasonic Echo Pin
#define FLOW_PIN     27  // Water Flow Sensor Pin (GPIO 17-க்கு பதிலாக GPIO 27)
#define BUZZER_PIN   13  // Alert Buzzer
#define RED_LED_PIN  2   // Danger Indicator LED
#define BLUE_LED_PIN 15  // Normal Indicator LED
#define GAS_PIN      34  // MQ-5 Gas Sensor Analog Pin

const int GAS_DANGER_LIMIT    = 500; 
const float WATER_FULL_MIN    = 0.1;  
const float WATER_FULL_MAX    = 10.0; 

volatile unsigned long pulseCount = 0;
unsigned long previousMillis = 0;

float flowRate = 0.0;
float distance = 0.0;
int gasValue = 0;

void IRAM_ATTR flowPulse() {
  pulseCount++;
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1.0;
  return (duration * 0.0343) / 2.0;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FLOW_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, HIGH);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("SMART DRAINAGE");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING SYSTEM");
  delay(2000);
  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulse, RISING);
  previousMillis = millis();
}

void loop() {
  distance = getDistance();
  gasValue = analogRead(GAS_PIN);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();
    flowRate = pulses / 7.5;
  }

  Serial.print("Water Depth: "); Serial.print(distance);
  Serial.print(" cm | Gas Level: "); Serial.print(gasValue);
  Serial.print(" | Flow: "); Serial.print(flowRate); Serial.println(" L/min");

  bool isGasHigh = (gasValue >= GAS_DANGER_LIMIT);
  bool isWaterFull = (distance >= WATER_FULL_MIN && distance <= WATER_FULL_MAX);

  if (isGasHigh) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!! DANGER ALERT !");
    lcd.setCursor(0, 1);
    lcd.print("HIGH GAS DETECT!");
    delay(1500);
  }
  else if (isWaterFull) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DRAINAGE STATUS:");
    lcd.setCursor(0, 1);
    lcd.print("WATER LEVEL FULL");
    delay(1500);
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    if (distance < 0) lcd.print("Lvl: ERR ");
    else { lcd.print("Lvl:"); lcd.print(distance, 1); lcd.print("cm "); }
    lcd.print("Gas:"); lcd.print(gasValue);

    lcd.setCursor(0, 1);
    lcd.print("STATUS: NORMAL");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Flow:"); lcd.print(flowRate, 1); lcd.print(" L/min");

    lcd.setCursor(0, 1);
    lcd.print("STATUS: NORMAL");
    delay(1500);
  }
}
