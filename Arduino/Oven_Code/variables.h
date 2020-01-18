/* LCD Library for 16x2 LCD */
#include <LiquidCrystal.h>

/* LCD Pins */
#define LCD_RS 4
#define LCD_RW 5
#define LCD_E 6
#define LCD_D4 7
#define LCD_D5 8
#define LCD_D6 9
#define LCD_D7 12

LiquidCrystal lcd(LCD_RS,LCD_RW,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7);

/* Digital Outputs */
#define PULSE_PIN 10 
#define BUZZER_PIN 11

/* Digital Inputs */
#define START_KEY 3
#define ABORT_KEY 2

/* Analog Inputs */
#define THERMO_HJ_PIN A0
#define THERMO_CJ_PIN A1
#define RAIL_5V A5  // Software adjust for reading

/* Constants */
#define AB_TEMP 50
#define TEMP_SOAK 150  //Set to 150C
#define TIME_SOAK 60 //Set to 60s
#define TEMP_REFL 217 //Set to 217C
#define TEMP_ABRT 235 //Set to 235C
#define TIME_REFL 45  //Set to 45s
#define TEMP_COOL 60  //Set to 60C

/* Oven Controller Variables */
int PWM = 0; // % PWM (0 - 100)
int pulse_on;     // Control uses time when to turn on the pulse (800 means turns on at 800 ms and off at 1000, ie PWM of 20%)
int state = 0; // Current State
// double lastmillis = 0;

/* Timing Variables */
int timer1_counter; // CLOCK SPEED
int Count1ms = 0;  // ms timer
int sec = 0; // second counter for state machine

/* Total Time */
int seconds = 0;
int minutes = 0;

/* Temperature Hot + Cold */
double Temp;  

/* Pulse output */
bool PULSE=0;

/* flags for start and abort control from buttons (1 means it is true) */
bool START=0;
bool ABORT=0;

/* some buzzer timer that does someting honestly i have no idea if i need it*/
int buzzer_ms=0;