/*

Lego EV3 Robot Project
Made by Gustaf Berg
gustaf.berg@student.hv.se
Created 2017-05-18
Edited 2017-05-19

*/

#include <stdio.h>
#include <unistd.h>

#include "ev3_lcd.h"
#include "ev3_command.h"
#include "ev3sensor.h"
#include "ev3_output.h"
#include "ev3_button.h"
#include "ev3_sound.h"

#define DEFAULTSPEED 10
#define SLOWSPEED 5
#define BACKWARDSPEED 20
#define ANGLE 100 /* Angle for the Mounting */

struct buttons
 {
  int Escape;
  int Enter;
  int Up;
  int Down;
  int Left;
  int Right;
  int MountIsUp;
 } aButton, *ButtonPtr;
 
struct buffer
 {
  char TextBuffer[50];
  char CalibrateBuffer[50];
 } aBuffer, *BufferPtr;

 struct calibration
 {
  int *Light;
 } aCalibration, *CalibrationPtr;

/* Function Prototypes */
void PressEscape(int *ButtonPtr);
void PressEnter(int *ButtonPtr);
void PressRight(int *ButtonPtr);
void FollowStraightLine(int *Calibration);
void Obstruction(void);
void ControlMounting(int *ButtonPtr);
void GoBackwards(void);
void UserMenu(int *BufferPtr);

int main(void)
 {
  /* Variable Declaration */
  aButton.Escape = 1;
  aButton.Enter = 0;
  aButton.Down = 1;
  aButton.Up = 1;
  aButton.MountIsUp = 1; /* Set to 1 if Robot has Mounting Up before start else
                           if down 0 */
  
  /* Initialization */
  OutputInit();
  initSensors();
  ResetAllTachoCounts(OUT_ABCD);
  setSensorMode(INPUT_3, COLOR_MODE_COLOR);
  LcdInit();
  ButtonLedInit();
  SoundInit();
  
  do
   {
    LcdClean();
    SetLedPattern(LED_GREEN);
    LcdText(0, 0, 10, "- Main Menu -");
    LcdText(1, 0, 30, "- Enter to Follow Line");
    LcdText(1, 0, 50, "- Up to Lower Mount");
    LcdText(1, 0, 60, "- Down to Raise Mount");
    LcdText(1, 0, 80, "- Left to Go Backwards");
    LcdText(1, 0, 100, "- Right to Calibrate");
    ControlMounting(&ButtonPtr);
    Off(OUT_BC);
    UserMenu(&BufferPtr);

    /* If left button is pressed go Backwards */
    if(ButtonIsDown(BUTTON_ID_LEFT))
     {
      GoBackwards();
     }
   }
  /* Infinite outer while-loop */
  while(1);

  return 0; /* Indicate successful ending */
 } /* End Main */

void UserMenu(int *BufferPtr)
 {
  /* Check if Enter, Escape or Right has been pressed */
  PressEnter(&ButtonPtr);
  PressEscape(&ButtonPtr);
  PressRight(&ButtonPtr);

  /* Enter while-loop if Enter is pressed and Escape has not been */
  while(aButton.Escape!=0 && aButton.Enter)
   {
    PressEscape(&ButtonPtr);
    LcdClean();
    LcdText(0, 0, 10, "- RUNNING -");
    LcdText(1, 0, 30, "- Escape to Exit");
    FollowStraightLine(&CalibrationPtr);
   }
  /* Enter while-loop if Right is pressed and Escape has not been */
  while(aButton.Escape!=0 && aButton.Right)
   {
    PressEscape(&ButtonPtr);
    LcdClean();
    LcdText(0, 0, 10, "- Calibration Menu -");
    LcdText(1, 0, 30, "- Escape to Exit");
    sprintf (aBuffer.TextBuffer, "- Sensor Value: %d", readSensor(INPUT_3));
    LcdText(1, 0, 50, aBuffer.TextBuffer);
    LcdText(1, 0, 60, "- Enter to Calibrate");
    /* Print Calibrated Value */
    LcdText(1, 0, 80, aBuffer.CalibrateBuffer);

    /* Calibrate the current Color-sensor Value with the Enter button */
    if(ButtonIsDown(BUTTON_ID_ENTER))
     {
      aCalibration.Light=readSensor(INPUT_3);
      sprintf (aBuffer.CalibrateBuffer, "- Calibrated: %d",
                                         readSensor(INPUT_3));
      LcdText(0, 0, 70, "- Calibrated");
      Wait(500);
     }
   }
 } /* End User-Menu */

/* This function check if the Escape button has been pressed down and returns 1
   else 0 */
void PressEscape(int *ButtonPtr)
 {
  if(ButtonIsDown(BUTTON_ID_ESCAPE))
   {
    aButton.Escape=0;
   }
  else
   {
    aButton.Escape=1;
   }
 }

/* This function check if the Enter button has been pressed down and returns 1
   else 0 */
void PressEnter(int *ButtonPtr)
 {
  if(ButtonIsDown(BUTTON_ID_ENTER))
   {
    aButton.Enter=1;
   }
  else
   {
    aButton.Enter=0;
   }
 }
 
/* This function check if the Enter button has been pressed down and return 1
   else 0 */
void PressRight(int *ButtonPtr)
 {
  if(ButtonIsDown(BUTTON_ID_RIGHT))
   {
    aButton.Right=1;
   }
  else
   {
    aButton.Right=0;
   }
 }

/* This function follow a straight line and check for obstacles */
void FollowStraightLine(int *Calibration)
 {
  if(readSensor(INPUT_3) > aCalibration.Light)
   {
    /* Turn Right */
    Obstruction();
    SetSpeed(OUT_B, DEFAULTSPEED);
    SetSpeed(OUT_C, SLOWSPEED);
    On(OUT_BC);
    SetLedPattern(LED_GREEN);
   }
  else if(readSensor(INPUT_3) <= aCalibration.Light)
   {
    /* Turn Left */
    Obstruction();
    SetSpeed(OUT_B, SLOWSPEED);
    SetSpeed(OUT_C, DEFAULTSPEED);
    On(OUT_BC);
    SetLedPattern(LED_RED);
   }
 }
 
/* This function check for obstacles and stops motors */
void Obstruction(void)
 {
  while(readSensor(INPUT_1) > 300)
   {
    SetLedPattern(LED_RED);
    LcdText(1, 0, 70, "Obstacle! Stopping.");
    Off(OUT_BC);
  }
 }

/* This function controls the mounting */
void ControlMounting(int *ButtonPtr)
 {
  /* Raise the Mounting if Down button is pressed and MountIsUp are false */
  if(ButtonIsDown(BUTTON_ID_DOWN) && aButton.MountIsUp==0)
   {
    RotateMotor(OUT_A, 20, ANGLE);
    aButton.MountIsUp=1;
   }
  /* Lower the Mounting if Up button is pressed and MountIsUp are true */
  else if(ButtonIsDown(BUTTON_ID_UP) && aButton.MountIsUp)
   {
    RotateMotor(OUT_A, -20, ANGLE);
    aButton.MountIsUp=0;
   }
 }
 
/* This function makes the robot go backwards */
void GoBackwards(void)
 {
  int Counter;
  for(Counter=0; Counter <= 50; Counter++)
   {
    //PressEscape(&ButtonPtr);
    if(aButton.Escape!=0)
     {
      LcdClean();
      LcdText(0, 0, 10, "- BACKING UP -");
      LcdText(1, 0, 30, "- Escape to Exit");
      SetSpeed(OUT_BC, -BACKWARDSPEED);
      On(OUT_BC);
      Wait(100);
      PressEscape(&ButtonPtr);
     }
   }
 }
