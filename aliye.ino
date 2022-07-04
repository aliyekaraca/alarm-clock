#include <DHT.h>
#include <LiquidCrystal.h>

#define DHTTYPE DHT11

#define B1_pin A0
#define B2_pin A1
#define B3_pin A2
#define B4_pin A4
#define dht 6
#define buz 7
#define led 9
#define pot A3


int B1_last = 0;
int B1_current;
unsigned long B1_hold;
int B2_last = 0;
int B2_current;
unsigned long B2_hold;
int B3_last = 0;
int B3_current;
unsigned long B3_hold;
int B4_last = 0;
int B4_current;
unsigned long B4_hold;

unsigned long resetMillis = 0;
unsigned long timeMillis;
unsigned long tickMillis;

unsigned long ringingMillis;
unsigned long alarmMillis;
unsigned long settingMillis;
unsigned long alarmBlink;
bool ring = true;
bool ringing = false;
bool alarmOn = false;
bool tempScale = false;
bool format24 = false;
int mode = 0;
//0 display date, check alarm
//1 set time min
//2 set time hour
//3 set alarm min
//4 set alarm hour

int alarmHour = 0;
int alarmMinute = 0;
int timeHour;
int timeMinute;
int newHour;
int newMinute;

DHT dht11(dht, DHTTYPE);
const int rs = 13, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
  Serial.begin(9600);
  dht11.begin();

  lcd.begin(16, 2);
  lcd.clear();
  
  byte degree[8] = {
    B11100,
    B10100,
    B11100,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  };
  byte alarm[8] = {
    B00000,
    B00100,
    B01110,
    B01110,
    B01110,
    B11111,
    B00100,
    B00000
  };
  byte blank[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  };
  lcd.createChar(0, degree);
  lcd.createChar(1, alarm);
  lcd.createChar(2, blank);

  pinMode(B1_pin, INPUT_PULLUP);
  pinMode(B2_pin, INPUT_PULLUP);
  pinMode(B3_pin, INPUT_PULLUP);
  pinMode(B4_pin, INPUT_PULLUP);
  pinMode(dht, INPUT);
  pinMode(buz, OUTPUT);
  pinMode(pot, INPUT);
  pinMode(led, OUTPUT);

  pinMode(d4, OUTPUT);
  pinMode(d5, OUTPUT);
  pinMode(d6, OUTPUT);
  pinMode(d7, OUTPUT);
  pinMode(rs, OUTPUT);
  pinMode(en, OUTPUT);

  tone(buz, 1760, 50);
}

void loop() {
  analogWrite(led, map(analogRead(pot), 0, 1023, 0, 255));

  if (tickMillis + 1000 < millis()) {
    timeMillis += 1000;
    tickMillis = millis();
  }
  if (timeMillis >= 86400000) timeMillis = 0;
  //timeMillis = millis();
  timeMinute = timeMillis / 60000 % 60;
  timeHour = timeMillis / 3600000 % 24;
  
  switch (mode) {
    case 0: //DISPLAY ------------------------------------------
      B1_current = digitalRead(B1_pin);
      if(B1_last == LOW && B1_current == HIGH) B1_hold = millis();
      else if(B1_last == HIGH && B1_current == LOW) {
        if (B1_hold + 1500 > millis()) SwitchFormat();
        else {
          newMinute = timeMinute;
          newHour = timeHour;
          mode = 1;
        }
      }
      B1_last = B1_current;
    
      B2_current = digitalRead(B2_pin);
      if(B2_last == LOW && B2_current == HIGH) B2_hold = millis();
      else if(B2_last == HIGH && B2_current == LOW) {
        if (B2_hold + 1500 > millis()) AlarmSwitch();
        else mode = 3;
      }
      B2_last = B2_current;
    
      B3_current = digitalRead(B3_pin);
      if(B3_last == LOW && B3_current == HIGH) {}
      else if(B3_last == HIGH && B3_current == LOW) TempScale();
      B3_last = B3_current;
    
      B4_current = digitalRead(B4_pin);
      if(B4_last == LOW && B4_current == HIGH) {}
      else if(B4_last == HIGH && B4_current == LOW) Snooze();
      B4_last = B4_current;

      DisplayTime();
      DisplayAlarm();
      break;

    case 1: //TIME MINUTE SETUP----------------------------------------
      B1_current = digitalRead(B1_pin);
      if(B1_last == LOW && B1_current == HIGH) {}
      else if(B1_last == HIGH && B1_current == LOW) mode = 2;
      B1_last = B1_current;
    
      B3_current = digitalRead(B3_pin);
      if(B3_last == LOW && B3_current == HIGH) {
        if (newMinute < 59) newMinute++;
        else newMinute = 0;
        B3_hold = millis();
      }
      else if(B3_last == HIGH && B3_current == LOW) B3_hold = 0;
      else if(B3_hold != 0 && B3_hold + 1000 < millis()) {
        if (newMinute < 59) newMinute++;
        else newMinute = 0;
      }
      B3_last = B3_current;

      DisplayAlarm();
      TimeSetup();
      break;

    case 2:
      B1_current = digitalRead(B1_pin);
      if(B1_last == LOW && B1_current == HIGH) {}
      else if(B1_last == HIGH && B1_current == LOW) {
        timeMillis = (newMinute * 60000) + (newHour * 3600000);
        mode = 0;
      }
      B1_last = B1_current;
    
      B3_current = digitalRead(B3_pin);
      if(B3_last == LOW && B3_current == HIGH) {
        if (newHour < 23) newHour++;
        else newHour = 0;
        B3_hold = millis();
      }
      else if(B3_last == HIGH && B3_current == LOW) B3_hold = 0;
      else if(B3_hold != 0 && B3_hold + 1000 < millis()) {
        if (newHour < 23) newHour++;
        else newHour = 0;
      }
      B3_last = B3_current;

      DisplayAlarm();
      TimeSetup();
      break;

    case 3: //ALARM MINUTE SETUP---------------------------------------
      B2_current = digitalRead(B2_pin);
      if(B2_last == LOW && B2_current == HIGH) {}
      else if(B2_last == HIGH && B2_current == LOW) mode = 4;
      B2_last = B2_current;
    
      B3_current = digitalRead(B3_pin);
      if(B3_last == LOW && B3_current == HIGH) {
        if (alarmMinute < 59) alarmMinute++;
        else alarmMinute = 0;
        B3_hold = millis();
      }
      else if(B3_last == HIGH && B3_current == LOW)
        B3_hold = 0;
      else if(B3_hold != 0 && B3_hold + 1000 < millis()) {
        if (alarmMinute < 59) alarmMinute++;
        else alarmMinute = 0;
      }
      B3_last = B3_current;

      DisplayTime();
      AlarmSetup();
      break;

    case 4: //ALARM HOUR SETUP-----------------------------------------
      B1_current = digitalRead(B1_pin);
      if(B1_last == LOW && B1_current == HIGH) {}
      else if(B1_last == HIGH && B1_current == LOW) {}
      B1_last = B1_current;
    
      B2_current = digitalRead(B2_pin);
      if(B2_last == LOW && B2_current == HIGH) {}
      else if(B2_last == HIGH && B2_current == LOW) mode = 0;
      B2_last = B2_current;
    
      B3_current = digitalRead(B3_pin);
      if(B3_last == LOW && B3_current == HIGH) {
        if (alarmHour < 23) alarmHour++;
        else alarmHour = 0;
        B3_hold = millis();
      }
      else if(B3_last == HIGH && B3_current == LOW) B3_hold = 0;
      else if(B3_hold != 0 && B3_hold + 1000 < millis()) {
        if (alarmHour < 23) alarmHour++;
        else alarmHour = 0;
      }
      B3_last = B3_current;

      DisplayTime();
      AlarmSetup();
      break;
  }

  
  DisplayTemp();
  delay(50);
}

void DisplayTime(){
  lcd.setCursor(2, 0);

  if(format24) {
      if (timeHour < 10) lcd.print(" "); 
      lcd.print(timeHour);
      lcd.print(":");
      if (timeMinute < 10) lcd.print("0");
      lcd.print(timeMinute);
      lcd.print("  ");
  }
  else {
    if (timeHour % 12 < 10) lcd.print(" ");
    lcd.print(timeHour % 12);
    lcd.print(":");
    if (timeMinute < 10) lcd.print("0");
    lcd.print(timeMinute);
    if (timeHour / 12.0 > 1) lcd.print("pm");
    else lcd.print("am");
  }
}

void SwitchFormat(){
  format24 = !format24;
}
void TimeSetup(){
  lcd.setCursor(2, 0);

  switch(mode) {
    case 1:
      if (settingMillis + 400 > millis()) {
        if(format24) {
          if (newHour < 10) lcd.print(" "); 
          lcd.print(newHour);
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          lcd.print("  ");
        }
        else {
          if (newHour % 12 < 10) lcd.print(" ");
          lcd.print(newHour % 12);
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          if (newHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else if (settingMillis + 500 > millis()) {
        if(format24) {
          if (newHour < 10) lcd.print(" "); 
          lcd.print(newHour);
          lcd.print(":");
          lcd.print("  ");
          lcd.print("  ");
        }
        else {
          if (newHour % 12 < 10) lcd.print(" ");
          lcd.print(newHour % 12);
          lcd.print(":");
          lcd.print("  ");
          if (newHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else settingMillis = millis();
      break;
    case 2:
      if (settingMillis + 400 > millis()) {
        if(format24) {
          if (newHour < 10) lcd.print(" "); 
          lcd.print(newHour);
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          lcd.print("  ");
        }
        else {
          if (newHour % 12 < 10) lcd.print(" ");
          lcd.print(newHour % 12);
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          if (newHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else if (settingMillis + 500 > millis()) {
        if(format24) {
          lcd.print("  ");
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          lcd.print("  ");
        }
        else {
          lcd.print("  ");
          lcd.print(":");
          if (newMinute < 10) lcd.print("0");
          lcd.print(newMinute);
          if (newHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else settingMillis = millis();
      break;
  }
}

void DisplayAlarm(){
  lcd.setCursor(1, 1);

  if (alarmOn) {
    byte alarm;
    if (alarmBlink + 800 > millis()) {
      alarm = 1;
    }
    else if (alarmBlink + 1000 > millis()) {
      alarm = 2;
    }
    else alarmBlink = millis();
    lcd.write(alarm);
    
    if ((timeHour == alarmHour && timeMinute == alarmMinute) || ringing) {
      ringing = true;
      
      if (alarmMillis + 500 > millis()) {
        if(format24) {
          if (alarmHour < 10) lcd.print(" "); 
          lcd.print(alarmHour);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          lcd.print("  ");
        }
        else {
          if (alarmHour % 12 < 10) lcd.print(" ");
          lcd.print(alarmHour % 12);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else if (alarmMillis + 1000 > millis()) {
        if(format24) { 
          lcd.print("  :    ");
        }
        else {
          if (alarmHour % 12 < 10) lcd.print(" ");
          lcd.print(alarmHour % 12);
          lcd.print(" ");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else alarmMillis = millis();

      if (ringingMillis + 300 > millis()) {
        if (ring) tone(buz, 1760, 200);
        ring = false;
      }
      else if (ringingMillis + 400 > millis()) {
        noTone(buz);
        ring = true;
      }
      else ringingMillis = millis();
      
      return;
    }
  }
  else lcd.write(1);

  if(format24) {
      if (alarmHour < 10) lcd.print(" "); 
      lcd.print(alarmHour);
      lcd.print(":");
      if (alarmMinute < 10) lcd.print("0");
      lcd.print(alarmMinute);
      lcd.print("  ");
    }
    else {
      if (alarmHour % 12 < 10) lcd.print(" ");
      lcd.print(alarmHour % 12);
      lcd.print(":");
      if (alarmMinute < 10) lcd.print("0");
      lcd.print(alarmMinute);
      if (alarmHour / 12.0 > 1) lcd.print("pm");
      else lcd.print("am");
    }
}

void AlarmSwitch(){
  if (alarmOn) alarmOn = ringing = !alarmOn;
  else alarmOn = !alarmOn;
  
}
void AlarmSetup() {
  lcd.setCursor(2, 1);

  switch(mode) {
    case 3:
      if (settingMillis + 400 > millis()) {
        if(format24) {
          if (alarmHour < 10) lcd.print(" "); 
          lcd.print(alarmHour);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          lcd.print("  ");
        }
        else {
          if (alarmHour % 12 < 10) lcd.print(" ");
          lcd.print(alarmHour % 12);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else if (settingMillis + 500 > millis()) {
        if(format24) {
          if (alarmHour < 10) lcd.print(" "); 
          lcd.print(alarmHour);
          lcd.print(":");
          lcd.print("  ");
          lcd.print("  ");
        }
        else {
          if (alarmHour % 12 < 10) lcd.print(" ");
          lcd.print(alarmHour % 12);
          lcd.print(":");
          lcd.print("  ");
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else settingMillis = millis();
      break;
    case 4:
      if (settingMillis + 400 > millis()) {
        if(format24) {
          if (alarmHour < 10) lcd.print(" "); 
          lcd.print(alarmHour);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          lcd.print("  ");
        }
        else {
          if (alarmHour % 12 < 10) lcd.print(" ");
          lcd.print(alarmHour % 12);
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else if (settingMillis + 500 > millis()) {
        if(format24) {
          lcd.print("  ");
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          lcd.print("  ");
        }
        else {
          lcd.print("  ");
          lcd.print(":");
          if (alarmMinute < 10) lcd.print("0");
          lcd.print(alarmMinute);
          if (alarmHour / 12.0 > 1) lcd.print("pm");
          else lcd.print("am");
        }
      }
      else settingMillis = millis();
      break;
  }
}

void TempScale(){
  tempScale = !tempScale;
}

void Snooze(){
  if (ringing) {
    alarmMinute += 5;
    if(alarmMinute > 59) {
      if(alarmHour < 23) alarmHour++;
      else alarmHour = 0;
      alarmMinute = alarmMinute % 59;
    }
    ringing = false;
  }
}

void DisplayTemp(){
  lcd.setCursor(11, 1);
  if (tempScale) lcd.print((String)(int)dht11.readTemperature() + "C");
  else lcd.print((String)(int)dht11.readTemperature(true)+ "F");
  lcd.write(byte(0));
}
