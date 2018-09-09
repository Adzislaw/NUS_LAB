/*
  Title:   PrinterControl
  Author:  Adrian Suszynski
  Date:    05-09-18
*/

#include<LiquidCrystal.h>
LiquidCrystal lcd(16, 17, 23, 25, 27, 29);

////////////////////////////////////////////////////

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
#error "Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

////////////////////////////////////////////////////

#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62

////////////////////////////////////////////////////

#define TEMP_BED_PIN       A14   // Analog Input
int Vo;
float R1 = 4700;
float logR2, R2, T;
float A = 1.484778004e-03,
      B = 2.348962910e-04,
      C = 1.006037158e-07;

////////////////////////////////////////////////////

#define BEEPER_PIN         37    // Buzzer pin

////////////////////////////////////////////////////

#define X_MIN_PIN           3    // Endstop pin X
#define Y_MIN_PIN          14    // Endstop pin Y
#define Z_MIN_PIN          18    // Endstop pin Z

int X_Min;
int Y_Min;
int Z_Min;

////////////////////////////////////////////////////

String s;

////////////////////////////////////////////////////

bool Beeper = false;
bool Motor_Enabled = false;
bool Homing = false;
bool Temp = false;
bool Endstop_check = false;

////////////////////////////////////////////////////

int x;
int y;
int z;

int k, p;

int Step_X = 0;
int Step_Y = 0;
int Step_Z = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);
  lcd.begin(20, 4);

  pinMode(BEEPER_PIN, OUTPUT);

  pinMode(X_MIN_PIN, INPUT_PULLUP);
  pinMode(Y_MIN_PIN, INPUT_PULLUP);
  pinMode(Z_MIN_PIN, INPUT_PULLUP);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);

  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);

  pinMode(TEMP_BED_PIN, INPUT);

  digitalWrite(X_ENABLE_PIN, HIGH); // Beginning turn off driver X
  digitalWrite(Y_ENABLE_PIN, HIGH); // Beginning turn off driver Y
  digitalWrite(Z_ENABLE_PIN, HIGH); // Beginning turn off driver Z

  Function_Print_LCD(0, 0, "X:");
  Function_Print_LCD(3, 0, "?");
  Function_Print_LCD(7, 0, "Y:");
  Function_Print_LCD(10, 0, "?");
  Function_Print_LCD(14, 0, "Z:");
  Function_Print_LCD(17, 0, "?");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  FunctionCheckEndstop();

  ////////////////////////////////////////////////////

  Vo = analogRead(TEMP_BED_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (A + B * logR2 + C * logR2 * logR2 * logR2));
  T =  T - 273.15;
  delay(50);
  String Print_Temp = String(T);
  Function_Print_LCD(2, 2, "Bed temp:");
  Function_Print_LCD(12, 2, Print_Temp);
  Function_Print_LCD(16, 2, "'");

  ////////////////////////////////////////////////////

  if ( Serial.available() > 0)
  {
    s = Serial.readStringUntil('\n');

    if (s.indexOf("Beeper") >= 0)
    {
      sscanf(s.c_str(), "Beeper %d", &Beeper);
    }
    else if (s.indexOf("Endstop_check") >= 0)
    {
      sscanf(s.c_str(), "Endstop_check %d", &Endstop_check);
      p = 0;
    }
    else if (s.indexOf("Temp") >= 0)
    {
      sscanf(s.c_str(), "Temp %d", &Temp);
      k = 0;
    }
    else if (s.indexOf("Motor_Enabled") >= 0)
    {
      sscanf(s.c_str(), "Motor_Enabled %d", &Motor_Enabled);
    }
    else
    {
      sscanf(s.c_str(), "X%d Y%d Z%d", &Step_X, &Step_Y, &Step_Z);

      if (Motor_Enabled)
      {
        FunctionPrintPosLCD(Step_X, Step_Y, Step_Z);

        if (Step_X < 0)
        {
          digitalWrite(X_DIR_PIN, LOW);
          Step_X = -Step_X;

        } else {
          digitalWrite(X_DIR_PIN, HIGH);
        }

        if (Step_Y < 0)
        {
          digitalWrite(Y_DIR_PIN, LOW);
          Step_Y = -Step_Y;

        } else {
          digitalWrite(Y_DIR_PIN, HIGH);
        }

        if (Step_Z < 0)
        {
          digitalWrite(Z_DIR_PIN, LOW);
          Step_Z = -Step_Z;

        } else {
          digitalWrite(Z_DIR_PIN, HIGH);
        }
      }
      else
      {
        Function_Print_LCD(0, 0, "Please turn on motor");
        Serial.println("Please turn on motor.");
      }
    }
  }

  ////////////////////////////////////////////////////

  if (Step_X > 0)
  {
    digitalWrite(X_STEP_PIN, HIGH);
    Step_X--;
  }
  if (Step_Y > 0)
  {
    digitalWrite(Y_STEP_PIN, HIGH);
    Step_Y--;
  }
  if (Step_Z > 0)
  {
    digitalWrite(Z_STEP_PIN, HIGH);
    Step_Z--;
  }
  delayMicroseconds(100);
  digitalWrite(X_STEP_PIN, LOW);
  digitalWrite(Y_STEP_PIN, LOW);
  digitalWrite(Z_STEP_PIN, LOW);
  delayMicroseconds(100);

  ////////////////////////////////////////////////////

  if (Beeper)
  {
    digitalWrite(BEEPER_PIN, HIGH);
  }
  else
  {
    digitalWrite(BEEPER_PIN, LOW);
  }

  ////////////////////////////////////////////////////

  if (Temp && k == 0)
  {
    Serial.println(Print_Temp);
    k = 1;
    delay(500);
  }
  else
  {
  }

  ////////////////////////////////////////////////////

  if (Endstop_check && p == 0)
  {
    Serial.print("X:");
    Serial.print(X_Min);
    Serial.print(" Y:");
    Serial.print(Y_Min);
    Serial.print(" Z:");
    Serial.println(Z_Min);
    p = 1;
    delay(500);
  }
  else
  {
  }
  ////////////////////////////////////////////////////

  if (Motor_Enabled)
  {
    Function_Motor_On();
    Function_Print_LCD(2, 4, "Motor Enabled: 1");
  }
  else
  {
    Function_Motor_Off();
    Function_Print_LCD(2, 4, "Motor Enabled: 0");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Function_Motor_On()
{
  digitalWrite(X_ENABLE_PIN, LOW); //turn on driver X
  digitalWrite(Y_ENABLE_PIN, LOW); //turn on driver Y
  digitalWrite(Z_ENABLE_PIN, LOW); //turn on driver Z
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Function_Motor_Off()
{
  digitalWrite(X_ENABLE_PIN, HIGH); //turn off driver X
  digitalWrite(Y_ENABLE_PIN, HIGH); //turn off driver Y
  digitalWrite(Z_ENABLE_PIN, HIGH); //turn off driver Z
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Function_Print_LCD(int i, int j, String StringToPrint)
{
  lcd.setCursor(i, j);
  lcd.print(StringToPrint);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void FunctionCheckEndstop()
{
  X_Min = digitalRead(X_MIN_PIN); //Endstop X
  Y_Min = digitalRead(Y_MIN_PIN); //Endstop Y
  Z_Min = digitalRead(Z_MIN_PIN); //Endstop Z

  String Print_End_X = String(X_Min);
  String Print_End_Y = String(Y_Min);
  String Print_End_Z = String(Z_Min);

  Function_Print_LCD(0, 1, "EX:");
  Function_Print_LCD(4, 1, Print_End_X);
  Function_Print_LCD(7, 1, "EY:");
  Function_Print_LCD(11, 1, Print_End_Y);
  Function_Print_LCD(14, 1, "EZ:");
  Function_Print_LCD(18, 1, Print_End_Z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void FunctionPrintPosLCD(int StepPrintX, int StepPrintY, int StepPrintZ)
{
  String Print_X = String(StepPrintX);
  String Print_Y = String(StepPrintY);
  String Print_Z = String(StepPrintZ);

  lcd.clear();
  Function_Print_LCD(0, 0, "X:");
  Function_Print_LCD(2, 0, Print_X);
  Function_Print_LCD(7, 0, "Y:");
  Function_Print_LCD(9, 0, Print_Y);
  Function_Print_LCD(14, 0, "Z:");
  Function_Print_LCD(16, 0, Print_Z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void FunctionGetTemp()
{
  Vo = analogRead(TEMP_BED_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (A + B * logR2 + C * logR2 * logR2 * logR2));
  T =  T - 273.15;

  return T;
}
