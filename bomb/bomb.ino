/******************************************************************************
 * Bomb                                                                       *
 *                                                                            *
 * Bruno R. Arico                                                             *
 * Pedro L. Pereira                                                           *
 * Raphael R. Gusmao                                                          *
 ******************************************************************************/

#include <Wire.h>
#include <math.h>
#include <LiquidCrystal.h>

#define BLUE_LED 3
#define GREEN_LED 7
#define GREEN_CHALL1 8
#define GREEN_CHALL2 9
#define GREEN_CHALL3 10
#define GREEN_CHALL4 11

typedef struct {
  int x;
  int y;
  int z;
  int temperature;
  int gx;
  int gy;
  int gz;
} accelerometer_values;

typedef struct {
  unsigned long int begin_time;
  unsigned long int end_time;
  long int now_time;
} timer_control;

const static int MPU = 0x68;

#define pot1 A1
#define pot2 A2
#define red_led A3

long rnd_interval1;
long rnd_interval2;

timer_control time_fuse;
int last_print = 0;
LiquidCrystal lcd(2, 4, 5, 6, 11, 12);


accelerometer_values get_accelerometer_values() {
  accelerometer_values values;
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // get data from sensor
  values.x = Wire.read() << 8 | Wire.read();
  values.y = Wire.read() << 8 | Wire.read();
  values.z = Wire.read() << 8 | Wire.read();
  values.temperature = Wire.read() << 8 | Wire.read();
  values.gx = Wire.read() << 8 | Wire.read();
  values.gy = Wire.read() << 8 | Wire.read();
  values.gz = Wire.read() << 8 | Wire.read();
  return values;
}

bool is_close_to(int value, int desired, int tolerance) {
  return abs(value - desired) < tolerance;
}

bool blue_light() {
  // Accelerometer challenge
  accelerometer_values accel = get_accelerometer_values();

  // Serial.print("AcX = "); Serial.print(accel.x);

  // Serial.print(" | AcY = "); Serial.print(accel.y);
   
  // Serial.print(" | AcZ = "); Serial.print(accel.z);
   
  // Serial.print(" | Tmp = "); Serial.print(accel.temperature/340.00+36.53);
   
  // Serial.print(" | GyX = "); Serial.print(accel.gx);

  // Serial.print(" | GyY = "); Serial.print(accel.gy);
   
  // Serial.print(" | GyZ = "); Serial.println(accel.gz);

  return is_close_to(accel.x, 11500, 4200)
         && is_close_to(accel.y, -2000, 4200)
         && is_close_to(accel.z, -12000, 4200);
}

bool red_light() {
  // Potentiometer challenge

  float pot1_value = map(analogRead(pot1), 0, 915, 0, 900);
  float pot2_value = map(analogRead(pot2), 0, 915, 0, 900);

  Serial.print("Pot1: ");Serial.print(pot1_value);Serial.print("\n");
  Serial.print("Pot2: ");Serial.print(pot2_value);Serial.print("\n\n");

  if (rnd_interval1*100 < pot1_value && pot1_value <= (rnd_interval1+1)*100
   && rnd_interval2*100 < pot2_value && pot2_value <= (rnd_interval2+1)*100) {
    digitalWrite(red_led, HIGH);
  } else {
    digitalWrite(red_led, LOW);
  }

  return true;
}

bool green_light() {
  // Unplugging a wire challenge
  int  chall1 = !digitalRead(GREEN_CHALL1),
       chall2 = !digitalRead(GREEN_CHALL2),
       chall3 = !digitalRead(GREEN_CHALL3),
       chall4 = !digitalRead(GREEN_CHALL4);

  // Serial.print("1: "); Serial.print(chall1);
  // Serial.print(" 2: "); Serial.print(chall2);
  // Serial.print(" 3: "); Serial.print(chall3);
  // Serial.print(" 4: "); Serial.println(chall4);

  return chall1 && chall2 && chall3 && !chall4;
}

bool yellow_light() {
  // Button challenge
  return true;
}

void timer_begin(unsigned long int end_value) {
  time_fuse.end_time = end_value;
  time_fuse.begin_time = millis();
}

int timer_end_check() {
  time_fuse.now_time = time_fuse.end_time - (millis()-time_fuse.begin_time);
  if(time_fuse.now_time <= 0) return true;
  else {
    time_parser();
    return false;
  }
}

void time_parser() {
  if (millis() -last_print > 1000){
    last_print = millis();
    lcd.clear();
    int time_raw = time_fuse.now_time/1000;
    int minutes = time_raw/60;
    int seconds = time_raw%60;
    lcd.print(minutes);
    lcd.print(":");
    lcd.println(seconds);
  }
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  // Initializes MPU
  lcd.begin(16, 2);
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); Wire.write(0);
  timer_begin(60000);
  byte error = Wire.endTransmission(false);
  if (error != 0) {
    Serial.write("WARNING: Accelerometer not found.");
  }

  // Potentiometer
  pinMode(3, OUTPUT);
  lcd.setCursor(3, 1);
  lcd.print(" LCD 16x2");
  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
  pinMode(red_led, OUTPUT);
  rnd_interval1 = random(0, 9);
  rnd_interval2 = random(0, 9);

  pinMode(GREEN_CHALL1, INPUT_PULLUP);
  pinMode(GREEN_CHALL2, INPUT_PULLUP);
  pinMode(GREEN_CHALL3, INPUT_PULLUP);
  pinMode(GREEN_CHALL4, INPUT_PULLUP);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}

void defuse() {
}

void loop() {
  timer_end_check();
  bool blue   = blue_light(),
       red    = red_light(),
       green  = green_light(),
       yellow = yellow_light();
  digitalWrite(3, blue ? HIGH : LOW);

  digitalWrite(BLUE_LED, blue ? HIGH : LOW);
  digitalWrite(GREEN_LED, green ? HIGH : LOW);

  if (blue_light()
      && red_light()
      && green_light()
      && yellow_light()) {
        defuse();
  }

  delay(500);
}

