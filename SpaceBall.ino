#include <LiquidCrystal.h>
LiquidCrystal lcd(52, 53, 51, 49, 47, 45);

// Set constants
const int Input_Red_Brightness    = A0;
const int Input_Red_DutyCycle     = A1;
const int Input_Green_Brightness  = A2;
const int Input_Green_DutyCycle   = A3;
const int Input_Blue_Brightness   = A4;
const int Input_Blue_DutyCycle    = A5;
const int Input_Frequency         = A6;
const int Input_MotorHorizontal   = A8;
const int Input_MotorVertical     = A9;
const int Output_MotorHorizontal  = 12;
const int Output_MotorVertical    = 11;
const int Output_Red_Brightness   = 6;
const int Output_Green_Brightness = 7;
const int Output_Blue_Brightness  = 8;
const int Max_Red_Brightness      = 195;
const int Max_Green_Brightness    = 205;
const int Max_Blue_Brightness     = 195;
const int Min_MotorHorizontal     = 35;
const int Min_MotorVertical       = 45;
const int Max_Motor               = 200;
const int syncTreshold            = 10;   // Range for dutyCycle sync
const float sineWave[256] = {0,1,1,2,4,6,8,10,12,15,19,22,26,30,34,39,44,49,55,60,67,73,79,86,93,101,108,116,124,133,141,150,159,168,178,187,197,207,217,228,238,249,260,271,282,293,305,316,328,340,351,363,375,388,400,412,424,437,449,462,474,487,499,512,525,537,550,562,575,587,600,612,624,636,649,661,673,684,696,708,719,731,742,753,764,775,786,796,807,817,827,837,846,856,865,874,883,891,900,908,916,923,931,938,945,951,957,964,969,975,980,985,990,994,998,1002,1005,1009,1012,1014,1016,1018,1020,1022,1023,1023,1024,1024,1024,1023,1023,1022,1020,1018,1016,1014,1012,1009,1005,1002,998,994,990,985,980,975,969,964,957,951,945,938,931,923,916,908,900,891,883,874,865,856,846,837,827,817,807,796,786,775,764,753,742,731,719,708,696,684,673,661,649,636,624,612,600,587,575,562,550,537,525,512,499,487,474,462,449,437,424,412,400,388,375,363,351,340,328,316,305,293,282,271,260,249,238,228,217,207,197,187,178,168,159,150,141,133,124,116,108,101,93,86,79,73,67,60,55,49,44,39,34,30,26,22,19,15,12,10,8,6,4,2,1,1,0,0};

// Set vars
int Red_Brightness          = 0;      // Red LED brightness
int Red_Brightness_Input    = 0;      // Red LED brightness input value from knob
int Red_Brightness_Output   = 0;      // Red LED target brightness (used for transitions in random mode)
int Red_Brightness_Set      = 0;      // Red LED brightness change tracker
int Red_DutyCycle           = 0;      // Red LED duty cycle: what part of the cycle set by Frequency is the LED on
bool Red_State              = true;   // Red LED state: on or off (used by duty cycle)
unsigned long Red_Time      = 0;      // Red LED time since last state change
bool Red_Change             = true;   // Red LED change tracker (only write to LED when changes are made)
int Green_Brightness        = 0;
int Green_Brightness_Input  = 0;
int Green_Brightness_Output = 0;
int Green_Brightness_Set    = 0;
int Green_DutyCycle         = 0;
bool Green_State            = true;
unsigned long Green_Time    = 0;
bool Green_Change           = true;
int Blue_Brightness         = 0;
int Blue_Brightness_Input   = 0;
int Blue_Brightness_Output  = 0;
int Blue_Brightness_Set     = 0;
int Blue_DutyCycle          = 0;
bool Blue_State             = true;
unsigned long Blue_Time     = 0;
bool Blue_Change            = true;
int Frequency               = 250;    // Frequency of all 3 LEDs dutyCycles (the loop in which an LED is ON/OFF)
int Frequency_Input         = 250;    // Frequency input value from knob
int Frequency_Output        = 250;    // Frequency target value (used for transitions in random mode)
int MotorHorizontal         = 0;
int MotorHorizontal_Input   = 0;
int MotorHorizontal_Output  = 0;
int MotorVertical           = 0;
int MotorVertical_Input     = 0;
int MotorVertical_Output    = 0;
int activePreset            = 0;      // Anything other than 0 means a preset is active - export presets using Bluetooth DigitalWrite 7 (Serial 38400)
bool syncMode               = true;   // Mode in which dutycycles are synced to the same value when they are close
bool syncing                = false;  // True when dutycycle values are being synced
bool sineMode               = false;  // Mode in which all values oscillate in a frequency determined by the corresponding knob (uses a precalculated sinewave to save on processing power)
float sinePointer_RB        = 0;      // Red LED brightness sinewave pointer
float sinePointer_GB        = 0;
float sinePointer_BB        = 0;
float sinePointer_F         = 0;
float sinePointer_MH        = 0;
float sinePointer_MV        = 0;
bool randomMode             = false;  // Mode in which random values are picked in a frequency determined by the corresponding knob
int random_RB               = -1;     // Red LED brightness random target value
int switchCounter_RB        = 2000;   // Red LED timer for holding random value
int transitionSpeed_RB      = 10;     // Red LED randomized transition speed (how fast random values will crossfade) - must be float if slower speeds are required
int random_GB               = -1;
int switchCounter_GB        = 2000;
int transitionSpeed_GB      = 10;
int random_BB               = -1;
int switchCounter_BB        = 2000;
int transitionSpeed_BB      = 10;
int random_F                = -1;
int switchCounter_F         = 2000;
int transitionSpeed_F       = 10;
int random_MH               = -1;
int switchCounter_MH        = 2000;
int transitionSpeed_MH      = 10;
int random_MV               = -1;
int switchCounter_MV        = 2000;
int transitionSpeed_MV      = 10;
unsigned long currentTime;

void setup() {

  // Set PWM
  TCCR1B = TCCR1B & 0xF8 | 0x01;
  TCCR4B = TCCR4B & 0xF8 | 0x01;

  // Initialize LCD screen
  lcd.begin(16, 2);
  lcd.print("   SpaceBall    "); 
  lcd.setCursor(0, 1);
  lcd.print("   activated    " );

  // Open Serial connections
  Serial.begin(38400);
  Serial1.begin(9600); Serial1.println("SpaceBall connected"); Serial1.flush();
  
}

void setRandomWithTransition(int &inputvalue, int &outputvalue, int &value, int &switchCounter, int maxSpeed, int &randomTarget, int randomMin, int randomMax, int &transitionSpeed, int maxTransitionSpeed) {
  if (outputvalue < randomTarget) {
    outputvalue += transitionSpeed;
    if (outputvalue > randomTarget) { outputvalue = randomTarget; }
  } else if (outputvalue > randomTarget) {
    outputvalue -= transitionSpeed;
    if (outputvalue < randomTarget) { outputvalue = randomTarget; }
  } else if (outputvalue == randomTarget || randomTarget == -1) {
    switchCounter += map(inputvalue, 0, 1023, 0, maxSpeed);
    if (switchCounter > 1024) {
      switchCounter = 0;
      randomTarget = random(randomMin, randomMax);
      transitionSpeed = random(1, map(inputvalue, 0, 1023, 1, maxTransitionSpeed));
    }
  }
  value = outputvalue;
}

void sineInput(int &inputvalue, int &value, int maxspeed, float &pointer, int minval, int maxval) {
  float cyclespeed = map(inputvalue, 0, 1023, 0, maxspeed)/100.00;
  pointer += cyclespeed; if (pointer > 256) { pointer -= 256; }
  value = map(sineWave[round(pointer)], 0, 1023, minval, maxval);
}

void getInput() {

  if (activePreset != 0) {

    if (activePreset == 1) { Red_Brightness = 194; Red_DutyCycle = 643; Green_Brightness = 204; Green_DutyCycle = 431; Blue_Brightness = 194; Blue_DutyCycle = 554; Frequency = 194; MotorHorizontal = 189; MotorVertical = 146; }
    if (activePreset == 2) { Red_Brightness = 195; Red_DutyCycle = 95; Green_Brightness = 205; Green_DutyCycle = 186; Blue_Brightness = 195; Blue_DutyCycle = 0; Frequency = 254; MotorHorizontal = 35; MotorVertical = 92; }
    if (activePreset == 3) { Red_Brightness = 0; Red_DutyCycle = 552; Green_Brightness = 183; Green_DutyCycle = 1023; Blue_Brightness = 179; Blue_DutyCycle = 0; Frequency = 238; MotorHorizontal = 164; MotorVertical = 165; }
    if (activePreset == 4) { Red_Brightness = 0; Red_DutyCycle = 234; Green_Brightness = 205; Green_DutyCycle = 1013; Blue_Brightness = 195; Blue_DutyCycle = 14; Frequency = 255; MotorHorizontal = 172; MotorVertical = 147; }
    if (activePreset == 5) { Red_Brightness = 194; Red_DutyCycle = 352; Green_Brightness = 202; Green_DutyCycle = 629; Blue_Brightness = 195; Blue_DutyCycle = 1010; Frequency = 47; MotorHorizontal = 190; MotorVertical = 45; }

  } else {
    
    // Read analog input
    Red_Brightness_Input = analogRead(Input_Red_Brightness);
    Red_DutyCycle = analogRead(Input_Red_DutyCycle);
    Green_Brightness_Input = analogRead(Input_Green_Brightness);
    Green_DutyCycle = analogRead(Input_Green_DutyCycle);
    Blue_Brightness_Input = analogRead(Input_Blue_Brightness);
    Blue_DutyCycle = analogRead(Input_Blue_DutyCycle);
    Frequency_Input = analogRead(Input_Frequency);
    MotorHorizontal_Input = analogRead(Input_MotorHorizontal);
    MotorVertical_Input = analogRead(Input_MotorVertical);

    if (randomMode == true) {

      setRandomWithTransition(Red_Brightness_Input, Red_Brightness_Output, Red_Brightness, switchCounter_RB, 130, random_RB, 0, Max_Red_Brightness, transitionSpeed_RB, 20);
      setRandomWithTransition(Green_Brightness_Input, Green_Brightness_Output, Green_Brightness, switchCounter_GB, 130, random_GB, 0, Max_Green_Brightness, transitionSpeed_GB, 20);
      setRandomWithTransition(Blue_Brightness_Input, Blue_Brightness_Output, Blue_Brightness, switchCounter_BB, 130, random_BB, 0, Max_Blue_Brightness, transitionSpeed_BB, 20);
      setRandomWithTransition(Frequency_Input, Frequency_Output, Frequency, switchCounter_F, 130, random_F, 0, 255, transitionSpeed_F, 20);
      setRandomWithTransition(MotorHorizontal_Input, MotorHorizontal_Output, MotorHorizontal, switchCounter_MH, 10, random_MH, 0, Max_Motor, transitionSpeed_MH, 5);
      setRandomWithTransition(MotorVertical_Input, MotorVertical_Output, MotorVertical, switchCounter_MV, 10, random_MV, 0, Max_Motor, transitionSpeed_MV, 5);

    } else if (sineMode == true) {
  
      sineInput(Red_Brightness_Input, Red_Brightness, 1000, sinePointer_RB, 0, Max_Red_Brightness);
      sineInput(Green_Brightness_Input, Green_Brightness, 1000, sinePointer_GB, 0, Max_Green_Brightness);
      sineInput(Blue_Brightness_Input, Blue_Brightness, 1000, sinePointer_BB, 0, Max_Blue_Brightness);
      sineInput(Frequency_Input, Frequency, 1000, sinePointer_F, 255, 0);
      sineInput(MotorHorizontal_Input, MotorHorizontal, 200, sinePointer_MH, Min_MotorHorizontal, Max_Motor);
      sineInput(MotorVertical_Input, MotorVertical, 200, sinePointer_MV, Min_MotorVertical, Max_Motor);
      
    } else {
  
      // Process direct input
      Red_Brightness = map(Red_Brightness_Input, 0, 1023, 0, Max_Red_Brightness);
      Green_Brightness = map(Green_Brightness_Input, 0, 1023, 0, Max_Green_Brightness);
      Blue_Brightness = map(Blue_Brightness_Input, 0, 1023, 0, Max_Blue_Brightness); 
      Frequency = map(Frequency_Input, 0, 1023, 255, 0);
      MotorHorizontal = map(MotorHorizontal_Input, 0, 1023, Min_MotorHorizontal, Max_Motor);
      MotorVertical = map(MotorVertical_Input, 0, 1023, Min_MotorVertical, Max_Motor);
  
      // Sync mode
      if (syncMode == true) {
        int avgDc = (Red_DutyCycle + Green_DutyCycle + Blue_DutyCycle) / 3;
        syncing = false;
        if (abs(avgDc - Red_DutyCycle) < syncTreshold && abs(avgDc - Green_DutyCycle) < syncTreshold && abs(avgDc - Blue_DutyCycle) < syncTreshold) {
          syncing = true;
          Green_DutyCycle = Red_DutyCycle;
          Blue_DutyCycle = Red_DutyCycle;
        }
      }
      
    }
  }

  /* Bluetooth input
   * 02         Toggle RandomMode
   * 03         Toggle SineMode
   * 04         Toggle SyncMode
   * 07         Export preset
   * 8-13       Presets
   */ 

  Serial1.flush();
  int ard_command = 0;
  int pin_num = 0;
  int pin_value = 0;
  char get_char = ' ';
  if (Serial1.available() > 0) {
    get_char = Serial1.read();
    if (get_char == '*') {
      ard_command = Serial1.parseInt();
      pin_num = Serial1.parseInt();
      pin_value = Serial1.parseInt();
      if (ard_command == 10){

        switch (pin_num) {
          case 2:
            randomMode = !randomMode;
            if (randomMode == true) { Serial1.println("RandomMode on"); Serial1.flush(); } else { Serial1.println("RandomMode off"); Serial1.flush(); }
            break;
          case 3:
            sineMode = !sineMode;
            if (sineMode == true) { Serial1.println("SineMode on"); Serial1.flush(); } else { Serial1.println("SineMode off"); Serial1.flush(); }
            break;
          case 4:
            syncMode = !syncMode;
            if (syncMode == true) { Serial1.println("SyncMode on"); Serial1.flush(); } else { Serial1.println("Syncmode off"); Serial1.flush(); }
            break;
          case 7:
            Serial.print("Red_Brightness = "); Serial.print(Red_Brightness); Serial.print("; Red_DutyCycle = "); Serial.print(Red_DutyCycle); Serial.print("; Green_Brightness = "); Serial.print(Green_Brightness); Serial.print("; Green_DutyCycle = "); Serial.print(Green_DutyCycle); Serial.print("; Blue_Brightness = "); Serial.print(Blue_Brightness); Serial.print("; Blue_DutyCycle = "); Serial.print(Blue_DutyCycle); Serial.print("; Frequency = "); Serial.print(Frequency); Serial.print("; MotorHorizontal = "); Serial.print(MotorHorizontal); Serial.print("; MotorVertical = "); Serial.print(MotorVertical); Serial.println(";");
            break;
          case 8:
            activePreset = 5; break;
          case 9:
            activePreset = 4; break;
          case 10:
            activePreset = 3; break;
          case 11:
            activePreset = 2; break;
          case 12:
            activePreset = 1; break;
          case 13:
            activePreset = 0; break;
        }
        if (pin_num > 8) { lcd.clear(); lcd.setCursor(0, 0); lcd.print("Preset "); lcd.setCursor(8, 0); lcd.print(activePreset); }
        
      }
    }
  }

}

void debugInputLeds_LCD() {
  lcd.setCursor(0, 0); lcd.print("R     "); lcd.setCursor(1, 0); lcd.print(Red_Brightness_Input);
  lcd.setCursor(6, 0); lcd.print("G     "); lcd.setCursor(7, 0); lcd.print(Green_Brightness_Input);
  lcd.setCursor(12, 0); lcd.print("B   "); lcd.setCursor(13, 0); lcd.print(Blue_Brightness_Input);
  lcd.setCursor(0, 1); lcd.print("      "); lcd.setCursor(0, 1); lcd.print(Red_DutyCycle);
  lcd.setCursor(6, 1); lcd.print("      "); lcd.setCursor(6, 1); lcd.print(Green_DutyCycle);
  lcd.setCursor(12, 1); lcd.print("    "); lcd.setCursor(12, 1); lcd.print(Blue_DutyCycle);
  if (syncing == true) { lcd.setCursor(4, 1); lcd.print("-"); lcd.setCursor(10, 1); lcd.print("-"); }
}

void debugInputMotors_LCD() {
  lcd.setCursor(0, 0); lcd.print("MH      "); lcd.setCursor(3, 0); lcd.print(MotorHorizontal_Input);
  lcd.setCursor(0, 1); lcd.print("MV      "); lcd.setCursor(3, 1); lcd.print(MotorVertical_Input);
}

void debugOutput(bool allvalues) {
  Serial.print(Red_Brightness); Serial.print(",");
  Serial.print(Green_Brightness); Serial.print(",");
  Serial.print(Blue_Brightness); Serial.print(",");
  Serial.print(Frequency); Serial.print(",");
  Serial.print(MotorHorizontal); Serial.print(",");
  Serial.print(MotorVertical);
  if (allvalues == true) {
    Serial.print(",");
    Serial.print(Red_DutyCycle); Serial.print(",");
    Serial.print(Green_DutyCycle); Serial.print(",");
    Serial.print(Blue_DutyCycle);
  }
  Serial.println("");
}

void toggleLed(String color, bool &state, unsigned long currentTime, unsigned long &ledTime, float dutyCycle, bool &change, int &brightness, int &brightness_set) {

  if (brightness == 1) { brightness = 0; }

  if (dutyCycle < 1020) {
    unsigned long toggleTime = currentTime - ledTime;
    if (state == false && toggleTime > (Frequency*(1024-dutyCycle)/1023)) {
      change = true; state = true;
      ledTime = currentTime;
    } else if (state == true && toggleTime > Frequency*(dutyCycle/1023)) {
      change = true; state = false;
      ledTime = currentTime;
      brightness = 0;
    }
  } else if (state == false) {
    change = true; state = true;
  }
  
  if (state == true && change == false && brightness_set < (brightness-1) || state == true && change == false && brightness_set > (brightness+1)) {
    brightness_set = brightness;
    change = true;
  }
   
}

void toggleLeds() {
  toggleLed("Red", Red_State, currentTime, Red_Time, Red_DutyCycle, Red_Change, Red_Brightness, Red_Brightness_Set);
  toggleLed("Green", Green_State, currentTime, Green_Time, Green_DutyCycle, Green_Change, Green_Brightness, Green_Brightness_Set);
  toggleLed("Blue", Blue_State, currentTime, Blue_Time, Blue_DutyCycle, Blue_Change, Blue_Brightness, Blue_Brightness_Set);
}

void debugLedStates() {
  Serial.print(Red_State); Serial.print(",");
  Serial.print(Green_State+3); Serial.print(",");
  Serial.println(Blue_State+6);
}

void writeValues() {
  if (Red_Change == true) { analogWrite(Output_Red_Brightness, Red_Brightness); Red_Change = false; }
  if (Green_Change == true) { analogWrite(Output_Green_Brightness, Green_Brightness); Green_Change = false; }
  if (Blue_Change == true) { analogWrite(Output_Blue_Brightness, Blue_Brightness); Blue_Change = false; }
  if (MotorHorizontal > 40) { analogWrite(Output_MotorHorizontal, MotorHorizontal); } else { analogWrite(Output_MotorHorizontal, 0); }
  if (MotorVertical > 50) { analogWrite(Output_MotorVertical, MotorVertical); } else { analogWrite(Output_MotorVertical, 0); }
}

void loop() {
  currentTime = millis();
  getInput();
  debugOutput(false);
  debugInputLeds_LCD();
  // debugInputMotors_LCD(); // disable debugInputLeds_LCD first
  toggleLeds();
  // debugLedStates(); // disable debugOutput first
  writeValues();
}
